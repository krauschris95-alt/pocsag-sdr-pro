param(
 [Parameter(Mandatory=$true)][string[]]$Files,
 [Parameter(Mandatory=$true)][string]$CertificateThumbprint,
 [Parameter(Mandatory=$true)][uri]$TimestampServer,
 [Parameter(Mandatory=$true)][string]$SignTool,
 [ValidateSet('CurrentUser','LocalMachine')][string]$Store='CurrentUser'
)
$ErrorActionPreference='Stop'
if($TimestampServer.Scheme -ne 'https'){throw 'Serveur RFC 3161 HTTPS requis'}
if(-not(Test-Path -LiteralPath $SignTool -PathType Leaf)){throw 'SignTool du SDK Windows absent'}
$thumb=$CertificateThumbprint.Replace(' ','').ToUpperInvariant()
if($thumb -notmatch '^[0-9A-F]{40}$'){throw 'Empreinte de certificat SHA-1 invalide'}
$cert=Get-Item -LiteralPath "Cert:\$Store\My\$thumb"
if(-not $cert.HasPrivateKey -or $cert.NotAfter -le (Get-Date) -or $cert.NotBefore -gt (Get-Date)){throw 'Certificat expiré, non encore valide ou sans clé privée accessible'}
if($cert.Subject -eq $cert.Issuer){throw 'Certificat autosigné refusé pour cette distribution'}
$eku=$cert.Extensions | Where-Object {$_.Oid.Value -eq '2.5.29.37'}
if(-not($eku.EnhancedKeyUsages | Where-Object {$_.Value -eq '1.3.6.1.5.5.7.3.3'})){throw 'Usage Code Signing requis'}
$chain=[System.Security.Cryptography.X509Certificates.X509Chain]::new()
$chain.ChainPolicy.RevocationMode='Online'
$chain.ChainPolicy.ApplicationPolicy.Add([System.Security.Cryptography.Oid]::new('1.3.6.1.5.5.7.3.3'))
try {if(-not $chain.Build($cert)){throw 'Chaîne de confiance/révocation du certificat non validée'}} finally {$chain.Dispose()}
$resolved=@();foreach($item in $Files){$file=(Resolve-Path -LiteralPath $item).Path;if([IO.Path]::GetExtension($file) -notin '.exe','.dll','.msi'){throw "Format non prévu : $file"};$resolved+=$file}
foreach($file in $resolved){
 $arguments=@('sign','/sha1',$thumb,'/s','My','/fd','SHA256','/tr',$TimestampServer.AbsoluteUri,'/td','SHA256','/d','POCSAG SDR Pro')
 if($Store -eq 'LocalMachine'){$arguments+='/sm'}
 & $SignTool @arguments $file
 if($LASTEXITCODE -ne 0){throw "Signature/horodatage échoué : $file"}
 & $SignTool verify /pa /all /v $file
 if($LASTEXITCODE -ne 0){throw "Vérification Authenticode échouée : $file"}
 $signature=Get-AuthenticodeSignature -LiteralPath $file
 if($signature.Status -ne 'Valid' -or -not $signature.TimeStamperCertificate){throw "Signature valide et horodatée non confirmée : $file"}
 Get-FileHash -LiteralPath $file -Algorithm SHA256
}
