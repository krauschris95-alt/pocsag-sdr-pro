# POCSAG SDR Pro — 0.1.7

Préversion Windows x64, C++20 / Qt 6.8.3 / SQLite. Réception SDR, démodulation, décodage POCSAG, historique, TXT et voix Windows intégrés, sans SDRSharp, PDW, câble audio virtuel ni service cloud.

## Installation

Télécharger le ZIP portable ou l’installateur depuis les [Releases](https://github.com/krauschris95-alt/pocsag-sdr-pro/releases). Extraire le ZIP puis ouvrir `POCSAG-SDR-Pro.exe`, ou utiliser l’installateur par utilisateur. Fermer l’ancienne version avant d’ouvrir la nouvelle. Les DLL Qt, SQLite, MinGW et les bibliothèques RTL/SoapyRTL/HackRF sont fournies ; Python et Visual Studio ne sont pas nécessaires pour utiliser l’application. Le pilote USB adapté reste nécessaire, notamment WinUSB pour RTL-SDR. Les exécutables ne sont pas signés.

La base et les préférences restent dans `%LOCALAPPDATA%/POCSAG-SDR-Pro/POCSAG-SDR-Pro/`. Migration additive : aucun effacement de l’historique, des capcodes ou des TXT existants. L’installateur ne supprime pas les données et journaux utilisateur à la désinstallation.

## Correction SQLite 0.1.7

Cause reproduite : les curseurs persistants des anciens tableaux conservaient une vue WAL pendant les écritures du thread TXT. Une insertion de réception pouvait ensuite échouer avec **SQLITE_BUSY_SNAPSHOT 517**, affiché comme `database is locked Unable to fetch row`. Le code précédent abandonnait le message après cet échec.

Les tableaux utilisent maintenant des lectures courtes et des pages libérées après chaque requête. Les transactions de réception commencent par `BEGIN IMMEDIATE`. Un message est placé dans une file locale atomique **avant** son insertion ; un verrou temporaire diffère son traitement et déclenche une reprise contrôlée. Un identifiant interne unique empêche de réinsérer le même message après redémarrage.

`incoming-pending/` contient les messages en attente ; `journal-receipts/` contient les positions d’écriture servant à reprendre les TXT sans doublons. Ces fichiers sont locaux et peuvent contenir des données privées ; ne pas les supprimer pendant une reprise. Les identifiants techniques restent internes et n’apparaissent pas dans les nouveaux TXT. Une panne disque peut empêcher la persistance ; elle est signalée et les messages encore en mémoire ne bénéficient alors pas d’une garantie après arrêt brutal.

## Deux journaux TXT indépendants

Ouvrir **Enregistrements → Journal** :

1. **Tous les messages — Toute la Belgique** : choisir le dossier principal, puis activer ON. Un fichier `Belgique-AAAA-MM-JJ.txt` est créé dans ce dossier pour chaque date civile belge. La date dépend d’Europe/Brussels, y compris les changements été/hiver. Le changement de jour n’arrête pas le SDR ; une reprise conserve le jour attribué à la réception.
2. **Capcodes enregistrés** : choisir directement un fichier permanent, par exemple `D:/POCSAG/Interventions.txt`, puis activer ON. Seuls les capcodes enregistrés, actifs et cochés **Journal TXT** y sont ajoutés. Aucun fichier quotidien n’est créé pour cette destination.

Les deux ON/OFF sont indépendants et également accessibles dans le menu Enregistrements. Les chemins persistent après redémarrage. Parcourir utilise les dialogues Windows ; Ouvrir lance le dossier ou le fichier. Les nouveaux fichiers sont créés sans effacement des anciens contenus.

Les nouvelles lignes ont uniquement ce format, en UTF-8 :

```text
0146035 2026-11-05 13:00:00 Texte complet du message
```

Capcode sur sept chiffres, date ISO, heure belge, message. Aucun UUID, fréquence, mode, vitesse, alias ou source SDR. Les retours de ligne incorporés sont représentés par `\n` et `\r` pour conserver une ligne par message. Les anciennes lignes ne sont pas converties, réécrites ou supprimées. Une destination identique n’est ajoutée qu’une fois pour un même message ; deux messages distincts au contenu identique restent deux lignes.

Les anciennes configurations de chemins individuels et profils sont conservées en base mais ne dirigent plus les nouveaux messages : les deux destinations ci-dessus les remplacent. Les anciennes écritures déjà en attente restent dans la file avec leur destination et leur contenu d’origine. Le classement des versions antérieures par sous-dossiers datés reste intact sur disque ; les nouveaux fichiers quotidiens sont créés directement dans le dossier maintenant choisi.

## Tableaux et historique

Disposition PDW conservée : thème bleu/noir, deux tableaux superposés, séparation initiale 60/40 déplaçable et mémorisée, défilements indépendants. Fréquence sélectionnée en cyan en haut à droite ; spectre/waterfall dans leur onglet séparé. Aucun compteur de messages dans l’interface principale.

**Ordre initial : anciens en haut, nouveaux en bas.** Le suivi automatique accompagne les nouvelles lignes seulement si le tableau était déjà en bas. Remonter pour lire l’historique suspend ce suivi ; revenir en bas le rétablit. Affichage permet aussi de le désactiver. Recherche, filtres, copie, export CSV, colonnes redimensionnables, tri volontaire et double-clic pour lire le texte intégral restent disponibles.

Aucun plafond applicatif du nombre de messages et aucune suppression automatique. La conservation dépend du stockage disponible. Les pages d’affichage sont chargées à la demande et le cache est borné à 4096 lignes par tableau ; cette borne concerne uniquement le cache, jamais l’historique SQLite accessible. Les anciennes options de conservation ne déclenchent aucune purge à la réception ; la purge manuelle exige toujours une confirmation explicite.

Le tableau supérieur affiche tous les messages acceptés sous réserve des filtres d’affichage. Le tableau inférieur affiche les capcodes actifs cochés **En bas**. Affichage bas, Journal TXT, Voix et préparation Pager sont indépendants. La fréquence est conservée dans SQLite et dans le tableau supérieur ; la colonne Source est optionnelle, sans ajout au TXT compact.

## Réception, décodage et qualité

Choisir le SDR et la fréquence réellement utilisée (169,626000 MHz sur le poste de qualification), puis Démarrer. Le voyant vert dépend d’IQ récents ; il reste rouge à l’arrêt ou sans flux. Il ne prouve pas qu’un signal POCSAG est présent.

**Qualité BCH** = proportion des mots POCSAG reçus acceptés après correction BCH et contrôle de parité. Aucun pourcentage n’est affiché sans mesure exploitable. Ce n’est ni un SNR ni une estimation des messages manqués ou de la couverture radio.

Débits AUTO/512/1200/2400, interprétation ALPHA/NUMERIC/AUTO globale et surcharge par capcode. Les fonctions 1 à 4 ne déterminent pas universellement le type de texte. ALPHA est le choix initial ; AUTO conserve les ambiguïtés comme diagnostics. TONE ONLY et diagnostics sont désactivés avant historique/TXT par défaut. Les filtres ALPHA/NUMERIC restent des filtres d’affichage. ASCII 7 bits pris en charge, sans promesse de déchiffrement ou de récupération universelle.

Le pas de fréquence agit sur les flèches, boutons et molette : 1, 2,5, 5, 6,25, 8,33, 10, 12,5, 25, 50 et 100 kHz, ou pas personnalisé en Hz. Un réaccord peut redémarrer brièvement la chaîne. FFT réelle, waterfall, marqueur rouge, zoom, contraste, vitesse, sélection à la souris et bande du canal sont conservés.

Le journal `technical.log` distingue l’activité IQ, les trames du décodeur, les insertions SQLite, la file de réception et les deux modèles. Les compteurs de diagnostic restent dans ce fichier, sans surcharger la fenêtre. Les erreurs TXT sont réessayées et n’arrêtent pas le décodeur ; la file SQLite reste persistante.

## Voix et matériel

SAPI Windows local : alias annoncé avant le message, choix de voix et sortie, volume, vitesse, ON/OFF général et par capcode, file ordonnée, test, relecture et arrêt. Une panne vocale ne doit pas interrompre le SDR ou les TXT.

RTL-SDR Blog V1.4.0 natif, SoapySDR 0.8.1 + SoapyRTL 0.3.3 et bibliothèque native HackRF distribués. Le RTL OEM présent avec tuner R828D est le matériel réellement disponible pour qualification. V3/V4 spécifiques et autres tuners ne sont pas tous qualifiés. HackRF est intégré mais non essayé physiquement.

Airspy/HF+, SDRplay, Lime, bladeRF, Pluto, USRP et Funcube nécessitent des modules/pilotes supplémentaires x64 compatibles avec l’ABI Soapy 0.8 et la chaîne MinGW ; ils ne sont pas inclus et ne sont pas déclarés testés. SoapySDR n’est pas une garantie universelle. `SOAPY_SDR_PLUGIN_PATH` et `POCSAG_SOAPY_LIBRARY` permettent des modules/bibliothèques compatibles de confiance.

Plusieurs sources ont leurs paramètres et décodeurs indépendants. Récepteur → Source du spectre choisit celle représentée ; les autres continuent. Arrêter coupe toutes les sources, Démarrer concerne le principal. Ne pas ouvrir le même appareil à la fois par deux adaptateurs. Deux lectures IQ simultanées sont testées ; deux appareils physiques restent à qualifier.

Limites : fréquence stockée sur 32 bits, maximum 4294,967295 MHz ; Soapy 120 kéch/s à 20 Méch/s selon pilote/CPU/USB, conversion CF32 vers le CU8 du moteur. Un module natif bloquant peut retarder l’arrêt. Diagnostic CPU x86-64/SSE2/AVX matériel disponible ; aucune instruction AVX obligatoire.

PrivateRadio reste une préparation hors ligne : préférences persistées et contrat de message, aucun transport, serveur, authentification ou relais actif. Aucun texte radio n’est envoyé au cloud.

## Compilation et preuves

Qt 6.8.3 / MinGW 13.1 x64, CMake, Ninja :

```powershell
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
cmake --install build --prefix dist/POCSAG-SDR-Pro
windeployqt --release --no-translations dist/POCSAG-SDR-Pro/POCSAG-SDR-Pro.exe
./scripts/package.ps1
python -m pip install backports.zstd
python scripts/package_hackrf.py dist/POCSAG-SDR-Pro hackrf-cache
python scripts/build_sdr_modules.py
python scripts/build_installer.py build-sdr-deps/nsis-source/nsis-3.12/makensis.exe 0.1.7
```

Lire [la qualification 0.1.7](docs/QUALIFICATION-0.1.7.md), [les rapports historiques](docs/VALIDATION.md) et [les licences tierces](docs/THIRD_PARTY.md). Distinguer tests automatiques, références indépendantes, captures privées et essais RF. Windows 10, AMD, autres éditions Windows et H24 restent à qualifier ; aucune compatibilité universelle annoncée.
