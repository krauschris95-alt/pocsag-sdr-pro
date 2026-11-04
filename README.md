# POCSAG SDR Pro — 0.1.2

Application Windows x64 C++20 / Qt 6 / SQLite, réception RTL-SDR intégrée, sans télémétrie ni transmission automatique des messages. Préversion en qualification, projet indépendant.

## Installation et réception

Télécharger le ZIP depuis les [Releases](https://github.com/krauschris95-alt/pocsag-sdr-pro/releases), extraire puis ouvrir `POCSAG-SDR-Pro.exe`. Qt, SQLite et RTL-SDR Blog V1.4.0 x64 sont inclus. Installer WinUSB pour le bon récepteur avec Zadig si nécessaire. La bibliothèque prend en charge V3/V4 ; seul le récepteur générique présent a été testé.

Choisir le SDR, la fréquence (169,626000 MHz sur le poste testé), le gain et les PPM, puis Démarrer. Fermer les applications occupant le même dongle. Réception : un canal centré, IQ 240 kS/s, débits AUTO/512/1200/2400.

Les changements de fréquence/gain/PPM/débit en direct redémarrent explicitement la chaîne SDR, avec une courte interruption possible. Le pas est réglable à 1, 2,5, 5, 6,25, 8,33, 10, 12,5, 25, 50 et 100 kHz, ou personnalisé en Hz. Boutons, flèches clavier et molette utilisent ce pas. La saisie directe se valide par Entrée ou perte de focus ; le pilote reçoit des Hz entiers arrondis.

Le repère rouge indique le centre accordé sur le spectre ET le waterfall, y compris au zoom. L'ancien spectre est effacé lors d'un changement de fréquence.

## Décodage et filtres

Les bits de fonction ne déterminent plus universellement le type de texte. Options → Interprétation des données :

- ALPHA : choix initial explicite, indépendant de la fonction.
- NUMERIC : chiffres et symboles POCSAG, dont `. U - ] [`.
- AUTO : mode conservateur ; une interprétation ambiguë reste diagnostique, sans texte inventé.

Chaque capcode peut surcharger ce mode ; INHERIT utilise le choix global. Les contrôles non pris en charge ne sont pas simplement supprimés : le contenu est classé diagnostic et représenté avec des balises hexadécimales. Une trame BCH correcte peut transporter un contenu binaire ou un codage non pris en charge. Aucun déchiffrement ou récupération universelle des signaux dégradés n'est promis.

Filtres → Types de messages propose ALPHA et NUMERIC activés, TONE ONLY désactivé, diagnostics désactivés. Le filtre agit avant SQLite et TXT. Les anciens TONE sont conservés dans la base migrée mais masqués ; les anciennes écritures en attente de types désormais désactivés sont annulées. Le compteur des messages texte exclut TONE et diagnostics. Les choix persistent et sont modifiables pendant la réception.

Les menus Fichier, Réception, Options, Filtres, Affichage, Capcodes, Journal et Aide agissent sur les fonctions réelles. Les tableaux conservent tri, recherche, copie, export CSV et ouverture du texte complet par double-clic. Le panneau inférieur contient exclusivement les messages des capcodes actifs choisis pour cet affichage.

## Toute la Belgique : journaux quotidiens

Activer Enregistrement TXT. Dans Journal, choisir le dossier principal avec Parcourir, le nom de profil (Belgique par défaut) et le classement quotidien. Exemple : racine `D:\Radio\POCSAG`, profil `Belgique` :

```text
D:\Radio\POCSAG\Belgique\2026-11-04\Belgique-2026-11-04.txt
D:\Radio\POCSAG\Belgique\2026-11-05\Belgique-2026-11-05.txt
```

La date civile est celle d'Europe/Brussels, avec changements été/hiver, indépendamment du fuseau Windows. Les dossiers intermédiaires sont créés automatiquement. La destination est fixée à la réception avant la mise en file SQLite : une écriture reprise le lendemain conserve le bon jour. Le passage à minuit n'arrête pas la réception. Le chemin actif est visible ; Ouvrir le dossier actif lance l'Explorateur Windows.

« Toute la Belgique » est un nom de flux configurable, sans garantie de provenance géographique ni de couverture nationale. Le mode permanent et les profils supplémentaires existants restent disponibles.

## Capcodes : dossiers indépendants

Capcodes permet de régler adresse (0–2097151, sept chiffres affichés), alias, couleur, Actif, En bas et interprétation. Pour chaque ligne, Parcourir choisit le dossier Windows ; Nom fichier TXT personnalise le nom (vide : adresse.txt). Quotidien crée des sous-dossiers datés dans ce dossier avec un fichier `nom-AAAA-MM-JJ.txt`.

Deux capcodes peuvent partager un fichier. Le chemin TXT hérité de 0.1.1 est utilisé si aucun dossier dédié n'est défini. L'import JSON accepte aussi l'ancien format. Enregistrer les modifications du dialogue.

Copie journal général conserve la copie générale en plus du fichier spécifique. Écriture en ajout UTF-8, sans effacement, via file SQLite persistante et verrou par fichier. Une même destination n'est ajoutée qu'une fois par message. Les erreurs sont affichées et réessayées toutes les dix secondes.

Historique (jours) règle la conservation SQLite par capcode (0 : illimitée). Appliquer la conservation enregistrée demande confirmation avant purge. Cette purge est manuelle et ne supprime jamais les TXT.

## Données, migration, mode hors ligne

La base existante `%LOCALAPPDATA%/POCSAG-SDR-Pro/POCSAG-SDR-Pro/pocsag.db` est conservée. Migration transactionnelle par ajout des colonnes manquantes. Filtres, dossiers, mode et pas sont dans `preferences.ini` à côté de la base.

Ouvrir IQ accepte du CU8 : I/Q non signés 8 bits, 240000 échantillons complexes/s, canal centré. Indiquer la fréquence avant ouverture pour les métadonnées ; un fichier brut ne les contient pas. Les captures privées ne sont jamais publiées automatiquement.

## Compilation et validation

Avec Qt 6.8.3 / MSVC 2022 x64 :

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64 -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cmake --install build --config Release --prefix dist/POCSAG-SDR-Pro
windeployqt --release --no-translations dist/POCSAG-SDR-Pro/POCSAG-SDR-Pro.exe
./scripts/package.ps1
```

GitHub Actions utilise MinGW x64 et inclut ses DLL pour un ZIP portable. Les tags `v*` publient une préversion après tests. Moteur seul : `-DBUILD_GUI=OFF`.

Voir [validation](docs/VALIDATION.md), [notes 0.1.2](docs/RELEASE-0.1.2.md), [licences](docs/THIRD_PARTY.md). Code original GPL-3.0-or-later.
