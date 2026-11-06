# Signature Windows et SmartScreen

La distribution actuelle est **non signée**. L’inspection ciblée des magasins de certificats Code Signing de l’utilisateur et de la machine n’a trouvé aucun certificat de signature utilisable sur ce poste. Aucun certificat n’a été créé, importé ou acheté ; aucune protection Windows n’a été modifiée. Les SHA-256 permettent de comparer le téléchargement au paquet publié, mais ne remplacent pas l’identité Authenticode d’un éditeur.

## Ce qui manque

Il faut une identité d’éditeur validée et un certificat Code Signing reconnu par Windows, avec sa clé privée accessible via le fournisseur du certificat (par exemple token/HSM ou service de signature), ou un service de signature reconnu avec validation d’identité. Un certificat autosigné ne fournit pas cette confiance publique. Les identifiants, clés et PIN doivent rester sous le contrôle de l’éditeur, jamais dans le dépôt.

La signature et la réputation SmartScreen sont deux contrôles distincts. Une signature reconnue permet d’identifier l’éditeur ; elle ne garantit pas la disparition immédiate du message « application non reconnue ». Microsoft indique que la réputation tient compte du fichier et de l’éditeur et qu’un certificat EV ne donne plus de contournement automatique. Voir la [documentation Microsoft SmartScreen](https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/smartscreen-reputation).

## Procédure préparée

1. Obtenir le certificat/service auprès du fournisseur retenu et installer son outil officiel avec l’accord de l’éditeur. Ne pas modifier les pilotes USB SDR dans cette opération.
2. Installer SignTool depuis le SDK Windows officiel. Choisir l’empreinte du certificat d’éditeur et le serveur d’horodatage RFC 3161 indiqué par le fournisseur.
3. Compiler et tester l’application, puis signer son EXE. Ne plus modifier l’EXE ensuite.
4. Construire l’installateur à partir de cet EXE signé, puis signer l’installateur.
5. Vérifier les deux signatures, constituer le ZIP et calculer les SHA-256 après toute signature. Publier les empreintes avec la préversion.

Le script [sign_windows.ps1](../scripts/sign_windows.ps1) exige explicitement les fichiers, l’empreinte du certificat, le chemin SignTool et le serveur HTTPS d’horodatage. Il refuse les certificats autosignés/expirés, vérifie l’usage Code Signing et la chaîne de confiance, puis effectue signature SHA-256, horodatage SHA-256 et vérifications. Il ne génère aucune identité et ne contient aucun secret. Exemple de paramètres à remplacer par ceux réellement fournis :

```powershell
./scripts/sign_windows.ps1 -Files ./dist/POCSAG-SDR-Pro/POCSAG-SDR-Pro.exe `
  -CertificateThumbprint $empreinteReelle -TimestampServer $serveurDuFournisseur `
  -SignTool $cheminDuSdk
# Construire ensuite l’installateur, puis le signer avec les mêmes paramètres.
Get-AuthenticodeSignature ./dist/POCSAG-SDR-Pro/POCSAG-SDR-Pro.exe
Get-FileHash ./dist/POCSAG-SDR-Pro-windows-x64.zip -Algorithm SHA256
```

Références : [SignTool et vérification Authenticode](https://learn.microsoft.com/en-us/windows/win32/seccrypto/signtool), [horodatage des signatures](https://learn.microsoft.com/en-us/windows/win32/seccrypto/time-stamping-authenticode-signatures). L’horodatage valide permet de vérifier une signature après l’expiration du certificat selon la politique Windows ; il n’est pas une garantie générale contre révocation ou compromission.

## Validation et limites

Le script est préparé pour un certificat réel ; aucune signature reconnue ne peut être validée en son absence. La livraison reste explicitement non signée avec empreintes vérifiables. Les essais d’ouverture de l’application ne constituent pas une qualification SmartScreen sur une installation Windows vierge ayant reçu le fichier par téléchargement. Windows 10 et Windows 11 doivent être qualifiés séparément ; ne pas conclure à deux OS testés à partir d’un seul poste. Ne jamais désactiver Defender, SmartScreen ou Smart App Control pour une validation.
