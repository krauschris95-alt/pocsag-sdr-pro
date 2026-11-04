# POCSAG SDR Pro — 0.1.0 en développement

Application Windows x64, C++20 / Qt 6 / SQLite. Projet indépendant.
**Cette version n'est pas une livraison complète du cahier des charges.**
Voir [la validation et les limites](docs/VALIDATION.md).

## Utilisation

Extraire l'archive portable dans un dossier accessible en écriture puis ouvrir `POCSAG-SDR-Pro.exe`.
Les données sont locales dans `%LOCALAPPDATA%/POCSAG-SDR-Pro/POCSAG-SDR-Pro/`.
Aucune télémétrie et aucun envoi des messages n'est implémenté.

Pour RTL-SDR, installer WinUSB pour le récepteur avec Zadig, après avoir sélectionné le bon
périphérique USB, puis placer la bibliothèque officielle RTL-SDR Blog x64 `rtlsdr.dll`
et ses dépendances à côté de l'exécutable. Le V4 requiert la version du pilote de RTL-SDR Blog.
Cliquer **Actualiser SDR**, choisir l'appareil, régler fréquence/gain/PPM et démarrer.
La réception utilise actuellement 240 kS/s et un canal centré. Les réglages sont appliqués
au démarrage de la réception. Une erreur USB arrête la chaîne et est signalée.

Le mode **Ouvrir IQ** accepte exclusivement du CU8 : octets I puis Q non signés,
240000 échantillons complexes/s, porteuse POCSAG centrée. Renseigner la fréquence
pour les métadonnées avant l'ouverture. Ce format brut ne contient pas ces métadonnées.

## Capcodes et journaux

**Capcodes** : ajouter l'adresse entre 0 et 2097151, l'alias, une couleur `#RRGGBB`,
les cases Actif et Afficher en bas, et éventuellement un chemin TXT. Enregistrer.
L'adresse est affichée sur sept chiffres. Les messages historiques sont immédiatement
refiltrés dans le panneau inférieur ; le tableau supérieur conserve tous les messages.
JSON permet l'import/export des réglages de capcodes.

**Enregistrement TXT** active les écritures UTF-8 en ajout, sans effacement.
**Copie journal général** conserve une copie générale lorsqu'un capcode possède un fichier
spécifique. Plusieurs capcodes peuvent partager le même chemin. Un même chemin exact
n'est écrit qu'une fois par message. Les erreurs TXT laissent le message dans SQLite.
Le dossier de destination doit déjà exister.

**Journal** gère les profils : nom, chemin, liste de capcodes séparés par virgules,
Actif (`1` ou `0`). Liste vide = tous les capcodes. Plusieurs profils peuvent être actifs.
Le fichier général peut être changé pendant la réception.

Les tableaux disposent de recherche, copie de lignes sélectionnées, export CSV et
ouverture du message complet par double-clic. Le panneau inférieur peut être masqué
et sa hauteur modifiée. Le défilement automatique peut être désactivé.

## Compilation

Installer Qt 6.8.3 Widgets/SQL et un compilateur C++20 compatible (MSVC 2022 x64 ou MinGW 13).

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64 -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cmake --install build --config Release --prefix dist/POCSAG-SDR-Pro
windeployqt --release --no-translations dist/POCSAG-SDR-Pro/POCSAG-SDR-Pro.exe
```

Le moteur seul peut être compilé avec `-DBUILD_GUI=OFF`.
Le workflow Windows génère un ZIP ; un tag `v*` déclenche une préversion GitHub Release.
Voir les [licences des dépendances](docs/THIRD_PARTY.md).
