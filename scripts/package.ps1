param([string]$Destination='dist/POCSAG-SDR-Pro')
$ErrorActionPreference='Stop'
New-Item -ItemType Directory -Force $Destination | Out-Null
$cache=Join-Path $env:TEMP ('pocsag-package-'+[guid]::NewGuid().ToString())
New-Item -ItemType Directory $cache | Out-Null
$binary=Join-Path $cache 'rtl.zip'
$source=Join-Path $cache 'rtl-source.zip'
Invoke-WebRequest 'https://github.com/rtlsdrblog/rtl-sdr-blog/releases/download/V1.4.0/Release.zip' -OutFile $binary
Invoke-WebRequest 'https://github.com/rtlsdrblog/rtl-sdr-blog/archive/refs/tags/V1.4.0.zip' -OutFile $source
if((Get-FileHash $binary).Hash -ne '7EF33F1304647F65E5E0FDE43637A73D54F076E91E651A3CECC4F55A17FD9815'){throw 'RTL binary checksum mismatch'}
if((Get-FileHash $source).Hash -ne 'EFAAC50D53EA2F12D9853167AC9470BF4F04DE944F17C5D52D2FD73A9EC24C18'){throw 'RTL source checksum mismatch'}
Expand-Archive $binary (Join-Path $cache 'rtl')
Copy-Item (Join-Path $cache 'rtl/x64/rtlsdr.dll'),(Join-Path $cache 'rtl/x64/pthreadVC2.dll') $Destination
New-Item -ItemType Directory -Force (Join-Path $Destination 'sources') | Out-Null
Copy-Item $source (Join-Path $Destination 'sources/rtl-sdr-blog-V1.4.0.zip')
Copy-Item README.md,LICENSE,docs/VALIDATION.md,docs/THIRD_PARTY.md $Destination
Copy-Item docs/RELEASE-0.1.11.md,docs/reference-validation-30s.json,docs/reference-validation-60s.json $Destination
Copy-Item licenses $Destination -Recurse -Force
# The temporary download cache is left for the OS to clean up; no recursive deletion.
Copy-Item docs/speech-validation-0.1.3.json,docs/radio-validation-0.1.3.json $Destination
Copy-Item docs/portable-validation-0.1.3.json $Destination
Copy-Item docs/portable-validation-0.1.4.json $Destination

Copy-Item docs/QUALIFICATION-0.1.11.md $Destination

New-Item -ItemType Directory -Force (Join-Path $Destination "docs") | Out-Null
Copy-Item docs/QUALIFICATION-0.1.11.md,docs/VALIDATION.md,docs/THIRD_PARTY.md (Join-Path $Destination "docs")
