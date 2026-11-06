## Version 0.1.19 — voix française

Codes P0 prononcés en français (P039 : « P zéro trente-neuf »), nombres ordinaires sans épellation systématique, pauses naturelles et message de test modifiable. Préparation identique en local et WAV PrivateRadio ; originaux conservés. Voir [qualification](docs/QUALIFICATION-0.1.19.md).

## Version 0.1.18 — échelle fixe du spectre et waterfall bleu

Le spectre utilise par défaut une échelle **fixe de −20 à −100 dBFS**, avec les deux bornes toujours graduées. Le mode automatique reste facultatif. Affichage → Réglages spectre / waterfall permet de régler la palette bleue, son contraste et sa luminosité, ainsi que le lissage temporel de la courbe cyan. Ces choix sont enregistrés immédiatement et restaurés au redémarrage.

La FFT et sa normalisation sont conservées. Le lissage est une moyenne exponentielle de puissance pour la courbe ; le waterfall garde les niveaux FFT bruts. Aucune translation artificielle du bruit de fond. Décodeur, capcodes, TXT, SQLite, voix et PrivateRadio inchangés. [Validation et captures réelles](docs/QUALIFICATION-0.1.18.md).

## Version 0.1.17 — adaptateurs USB et diagnostic Windows

Récepteur → **SDR détectés et pilotes…** sépare les appareils annoncés par une API des interfaces USB Windows. Actualiser, capacités, essai IQ de cinq secondes et choix du récepteur sont accessibles dans cette fenêtre. Une série absente ou ambiguë ne doit pas sélectionner arbitrairement une autre clé.

Ajout des API natives Airspy R2/Mini et HF+ (DLL x64 fournies) et d’un adaptateur expérimental FUNcube HID + audio IQ stéréo Windows. HackRF et Soapy conservés ; les runtimes Soapy externes et leurs modules associés peuvent être configurés pour SDRplay/Lime/bladeRF/Pluto selon les pilotes installés. Cela ne constitue pas une qualification physique de ces matériels.

[Compatibilité et pilotes](docs/SDR-COMPATIBILITY.md), [tests et limites](docs/QUALIFICATION-0.1.17.md), [signature Windows / SmartScreen](docs/WINDOWS-SIGNING.md). Distribution non signée faute de certificat d’éditeur ; empreintes SHA-256 publiées avec les téléchargements. Aucun pilote n’est installé ou remplacé automatiquement.

## Version 0.1.16 — fréquence cliquable et annonce P0

Cliquer sur l’encadré de fréquence en haut à droite, ou lui donner le focus puis Entrée/Espace : **Changer la fréquence**. Saisie MHz à six décimales, point ou virgule, flèches avec pas exact en Hz, quatorze pas prédéfinis et pas personnalisé. Six raccourcis, favoris supplémentaires, noms, ajout/modification/suppression et ordre mémorisés. Les flèches et favoris préparent une valeur ; **Appliquer** réaccorde le récepteur sélectionné, vérifie le pilote puis la reprise des IQ. En cas de refus, restauration de l’ancien accord lorsque possible, sinon arrêt avec erreur. Récepteur arrêté : préparation du prochain démarrage, sans activation. Aucune grille de canaux imposée.

La voix locale et les nouveaux WAV utilisent la même préparation : **alias → premier code autonome `P0xx` → suite entière**, chiffres prononcés séparément. Exemple : `APPEL: 50263090555 MED, P002 - AGRESSION - BAGARRE, N5 0` devient « départ ambulance R-MED1. P zéro zéro deux, agression, bagarre, N cinq, zéro ». Sans P0 valide, alias puis message complet. Les messages ambigus/incomplets/binaires restent exclus de l’audio automatique. Voix et Pager restent indépendants ; aucun raccordement au serveur officiel ajouté.

Les textes des tableaux, SQLite, TXT, profils, exports et le champ `text` JSON restent complets. Seul `utterance` contient la préparation vocale. Les anciens WAV ne sont pas réécrits. Thèmes Glacier/sombre, capcodes et historique conservés. [Qualification, captures et limites 0.1.16](docs/QUALIFICATION-0.1.16.md).

## Version 0.1.15 — Clair Professionnel — Bleu Glacier

Nouveau thème clair fidèle à la maquette : deux tableaux à titres bleus, fond blanc alterné bleu pâle, alias colorés, état SDR/BCH et fréquence à droite. Les fenêtres secondaires utilisent aussi le thème clair. Dans **Affichage → Thème**, le sombre reste disponible ; **Taille du texte** propose Compacte, Normale et Grande. Les deux choix sont mémorisés. Le clair devient le défaut des configurations qui ne contiennent pas encore de choix de thème.

Le décodeur, le filtrage, les journaux, l’historique et les réglages existants sont conservés. [Captures et qualification 0.1.15](docs/QUALIFICATION-0.1.15.md).

## Correctif 0.1.14 — nouveaux messages visibles

Les deux tableaux reprennent toujours l’ordre d’arrivée au démarrage. Le tri d’une ancienne session ne peut plus ranger silencieusement les nouvelles réceptions par texte ou par heure. Le bouton flèche vers le bas « Derniers messages » restaure cet ordre et rejoint le bas du tableau ; la consultation manuelle reste possible. Voir [qualification 0.1.14](docs/QUALIFICATION-0.1.14.md).

# POCSAG SDR Pro — 0.1.13

Préversion Windows x64, C++20 / Qt 6.8.3 / SQLite. Réception SDR, démodulation, décodage POCSAG, historique, TXT et voix Windows intégrés, sans SDRSharp, PDW, câble audio virtuel ni service cloud.

## Installation

Télécharger le ZIP portable ou l’installateur depuis les [Releases](https://github.com/krauschris95-alt/pocsag-sdr-pro/releases). Extraire le ZIP puis ouvrir `POCSAG-SDR-Pro.exe`, ou utiliser l’installateur par utilisateur. Fermer l’ancienne version avant d’ouvrir la nouvelle. Les DLL Qt, SQLite, MinGW et les bibliothèques RTL/SoapyRTL/HackRF sont fournies ; Python et Visual Studio ne sont pas nécessaires pour utiliser l’application. Le pilote USB adapté reste nécessaire, notamment WinUSB pour RTL-SDR. Les exécutables ne sont pas signés.

La base et les préférences restent dans `%LOCALAPPDATA%/POCSAG-SDR-Pro/POCSAG-SDR-Pro/`. Migration additive : aucun effacement de l’historique, des capcodes ou des TXT existants. L’installateur ne supprime pas les données et journaux utilisateur à la désinstallation.

## Interprétation ALPHA / NUMERIC — 0.1.13

Récepteur → Interprétation des données définit la politique réseau. Capcodes → Paramètres avancés permet **AUTO / ALPHA / NUMERIC / TONE**, indépendamment pour chaque capcode ; INHERIT conserve la politique réseau. Les choix sont mémorisés. Le défaut d'une nouvelle configuration est AUTO ; les anciens choix ALPHA/NUMERIC restent conservés.

**AUTO utilise une sélection heuristique documentée, pas une preuve du format applicatif.** Un texte ALPHA syntaxiquement valide et plausible est affiché et annoncé selon les interrupteurs habituels ; l'alternative NUMERIC et l'incertitude restent dans le diagnostic. Les charges compactes longues sans mots, à contrôles ou compatibles Base64 restent AMBIGUOUS et ne déclenchent pas de voix/WAV. Les chaînes Base64 sont analysées sans remplacement du texte original ni déchiffrement supposé.

Une règle ALPHA/NUMERIC/TONE explicite du capcode est prioritaire. AUTO ou INHERIT sur un capcode consulte la politique réseau globale ; si celle-ci est AUTO, la sélection heuristique s'applique. NUMERIC explicite reste disponible pour les émetteurs connus. Un rendu numérique valide ne prouve jamais à lui seul le type transmis. Les fonctions 0–3 n'imposent aucun type. Les capcodes et leurs interrupteurs ne sont pas réécrits.

NUMERIC prend en charge les chiffres et `. U espace - ] [` à 512/1200/2400 bauds. Le remplissage numérique final par espaces est retiré ; le protocole ne permet pas de distinguer un espace final intentionnel du remplissage. Les bits bruts restent disponibles. Les chiffres sont prononcés séparément, tandis que les TXT et le champ texte Pager conservent le contenu décodé exact.

Double-cliquer sur un message ouvre son texte complet, les candidats ALPHA/NUMERIC, la politique appliquée, les bits utiles corrigés, les mots radio avant correction, la fonction et le diagnostic. Une perte de synchronisation, un mot non corrigeable ou une fin de flux conserve le préfixe reçu comme INCOMPLETE, sans voix/WAV automatique. Les données non textuelles ne sont plus supprimées à l'ingestion par le filtre Diagnostics. Les champs absents sont expliqués. Les anciens enregistrements sans brut indiquent « Données non disponibles pour cet ancien enregistrement » ; leurs données radio ne peuvent pas être reconstituées. Enregistrements → Emplacement des données SQLite → Diagnostic de la connexion interroge la connexion réellement ouverte : PID, exécutable, fichiers, PRAGMA database_list, nombre de lignes et recherche UUID, sans écriture SQL.

Validation : six vecteurs IQ externes ALPHA/NUMERIC concordent avec multimon-ng, tous les débits et symboles numériques étendus. Une capture RTL réelle de 60 secondes donne 49 correspondances exactes dans chaque interprétation ; une trame supplémentaire reste non corroborée. Cela ne prouve pas qu'un émetteur réel utilise NUMERIC ni que les données compactes sont chiffrées. Voir le [rapport et les limites 0.1.13](docs/QUALIFICATION-0.1.13.md).

## Voix automatique et audio PrivateRadio — 0.1.11

Voix autorisée par défaut : **Actif + Voix ON** sur le capcode. Le bouton général est une coupure volontaire mémorisée. Alias, pause de 300 ms puis texte complet. Sortie SAPI réouverte par annonce ; changement de sortie Windows ou échec déclenche une réinitialisation contrôlée. Voix française de secours si le jeton précédent a disparu. Outils → Synthèse vocale propose les réglages et **Rétablir la voix française et la sortie Windows par défaut**.

Le diagnostic local indique la base réellement utilisée, les interrupteurs, l’éligibilité, la file, le début/fin et les erreurs COM, sans texte privé des interventions. La sortie Windows est identifiée avec son état muet et son volume ; la mesure de qualification distingue le signal réel du seul succès SAPI.

**Pager ON** sur un capcode actif génère un **vrai WAV PCM et son manifeste JSON**, même avec Voix locale OFF. Outils → PrivateRadio → **Dossier des annonces vocales** : dossier, vérification d’écriture et expiration de diffusion. UUID, contrôle puis publication atomique, file SQLite persistante et reprise. TXT et tableaux restent indépendants.

Relais distant **OFF par défaut**. L’application ne contacte aucun serveur officiel, même avec l’intention de relais ON : elle attend un adaptateur raccordé. L’adaptateur HTTPS est testé sur un serveur LAB isolé ; il ne constitue pas l’intégration PrivateRadio 3.0. Voir le [contrat](docs/PRIVATERADIO-AUDIO-CONTRACT.md) et le [rapport 0.1.11](docs/QUALIFICATION-0.1.11.md).

Les WAV/manifestes contiennent les messages privés : choisir un dossier réservé aux comptes autorisés. Aucun audio utilisateur publié. Aucun nettoyage automatique ; surveiller l’espace disque et utiliser la migration explicite des données si nécessaire.

## Corrections 0.1.10

Dans **Capcodes**, chaque ligne possède un **Fichier TXT** éditable, **Parcourir…** et **Ouvrir**. Plusieurs capcodes peuvent partager un fichier permanent ; les fichiers existants sont complétés sans être effacés. **Actif** et **Journal TXT** doivent être ON pour cette destination. Le fichier permanent commun de 0.1.9 reste disponible pour les lignes sans chemin individuel.

**Enregistrements → Profils TXT** permet de créer plusieurs destinations nommées, de les activer et de cocher leurs capcodes dans une liste recherchable par numéro ou alias. Un capcode peut appartenir à plusieurs profils. Les destinations identiques sont dédupliquées pour chaque événement ; deux messages réellement reçus identiques restent deux événements. Supprimer un profil ne supprime aucun fichier. Les anciens chemins historiques sont conservés sans être activés automatiquement.

Le journal **Belgique** reste indépendant, avec un fichier quotidien selon la date de Bruxelles. Tous les nouveaux TXT utilisent exclusivement `CAPCODE JJ/MM/AAAA HH:mm:ss MESSAGE`, en UTF-8, sans conversion des anciens fichiers. Une destination inaccessible reste en attente durable ; les autres fichiers continuent à être servis, même au-delà de 1 000 écritures en attente.

La voix est autorisée par défaut : **Actif + Voix ON** sur un capcode suffit. L’alias est annoncé avant le message complet, dans une file indépendante. **VOIX COUPÉES** est une coupure générale explicite et mémorisée ; une voix française installée est sélectionnée automatiquement au premier usage. Les réglages de sortie, volume et vitesse restent dans Outils. Le test sonore local a été mesuré sur la sortie Windows et confirmé audible par l’utilisateur.

Le tableau inférieur joint les capcodes numériquement, y compris les anciens numéros stockés en texte avec des zéros initiaux. Il reste indépendant des filtres généraux du tableau supérieur. Son filtre propre est signalé et peut être effacé directement. Les options **Actif**, **En bas** et les types de messages restent applicables.

L’historique SQLite n’est ni plafonné ni purgé automatiquement. Les identifiants et offsets sont sur 64 bits. Une fenêtre de consultation bornée en mémoire affiche jusqu’à un million de lignes ; le menu contextuel donne accès aux fenêtres précédentes/suivantes, au début et aux derniers messages. La limite de fenêtre ne supprime aucun message.

**Enregistrements → Emplacement des données** propose une migration explicite : arrêter la réception, créer une sauvegarde sur le disque cible, copier et contrôler les empreintes et l’intégrité SQLite, puis activer le nouveau dossier au redémarrage. L’original reste intact. Le dossier cible doit être vide ; aucun TXT externe n’est déplacé. Un échec laisse l’original actif. La réactivation explicite du dossier précédent ne fusionne pas les nouveaux historiques. Un disque de données absent provoque une erreur, jamais un retour silencieux au dossier par défaut. `--data-directory CHEMIN` permet un choix explicite de récupération.

Voir le [rapport de qualification 0.1.10](docs/QUALIFICATION-0.1.10.md). Les décodeurs SDR/POCSAG et les préférences PrivateRadio restent inchangés ; aucune transmission PrivateRadio n’est ajoutée.

## Sélection des destinations TXT — 0.1.9

**Enregistrements → Journal** présente deux destinations indépendantes : dossier Belgique quotidien et fichier permanent des capcodes. Chaque chemin est modifiable directement. Les grands boutons **Parcourir…** et **Choisir mon fichier TXT…** demandent le sélecteur Windows natif. Un fichier existant est ouvert en ajout ; un nom nouveau peut être saisi. Cliquez **Enregistrer** pour valider et utiliser les chemins.

Les lettres de disque accessibles et chemins UNC (`\\serveur\partage\Interventions.txt`) ne sont pas limités au disque C:. Les supports et partages doivent être disponibles et autorisés par Windows. Le logiciel ne monte pas de partage et ne demande pas d’identifiants réseau. Le bouton **CONFIGURER LE FICHIER TXT DES CAPCODES…** dans Capcodes ouvre directement la section du fichier permanent, sans perdre la saisie des capcodes.

La validation teste réellement l’écriture dans les dossiers et l’ouverture des fichiers en ajout, sans réécrire les anciens contenus. Les chemins sont enregistrés immédiatement dans les préférences. Les erreurs indiquent la destination et conservent la saisie ainsi que les destinations actives précédentes. Aucun repli automatique vers le dossier par défaut. Les anciennes écritures en attente gardent leur destination d’origine ; les nouvelles arrivées utilisent les chemins validés.

Voir le [rapport 0.1.9](docs/QUALIFICATION-0.1.9.md). Le décodeur, le filtrage des tableaux et le moteur d’écriture TXT sont inchangés.

## Corrections 0.1.8

Les interrupteurs **Actif**, **En bas**, **Voix**, **Journal TXT** et **Pager PrivateRadio** affichent explicitement ON/OFF dans la gestion des capcodes. Les réglages secondaires sont accessibles par **Paramètres avancés du capcode** ; les anciennes valeurs restent conservées. Le relais PrivateRadio demeure une préférence sans transmission réseau.

La barre supérieure propose **Voix ON/OFF**. Le menu Outils conserve le choix de voix Windows, volume, vitesse, test et arrêt ; le menu contextuel des tableaux permet de relire un message. Chaque lecture automatique annonce l’alias, puis le message complet, dans une file indépendante de la réception. Depuis 0.1.10, la voix générale est autorisée par défaut ; seule une coupure explicite la désactive.

**Derniers messages** ramène le tableau actif au bas, dans l’ordre de réception. Les tableaux restent indépendants. Les largeurs des colonnes, la géométrie et le séparateur sont mémorisés. Les dates affichées et les nouvelles lignes TXT utilisent `JJ/MM/AAAA` ; les noms quotidiens restent `Belgique-AAAA-MM-JJ.txt` et les anciens TXT ne sont pas convertis.

La jauge affiche le taux mesuré d’acceptation BCH/parité depuis la connexion, seulement pendant les transmissions récentes (expiration après trois secondes sans nouveaux mots). Ce n’est pas une estimation de couverture RF ou du taux de messages captés. Aucun RSSI/SNR ou état de synchronisation n’est inventé. Un avertissement technique signale moins de 256 Mio disponibles sur les volumes utilisés ; aucune purge automatique n’est lancée.

Voir le [rapport 0.1.8](docs/QUALIFICATION-0.1.8.md) pour les preuves et limites.

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
0146035 05/11/2026 13:00:00 Texte complet du message
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

Limites : fréquence stockée sur 32 bits, maximum 4294,967295 MHz ; Soapy 48 kéch/s à 20 Méch/s selon pilote/CPU/USB, conversion CF32 vers le CU8 du moteur. Un module natif bloquant peut retarder l’arrêt. Diagnostic CPU x86-64/SSE2/AVX matériel disponible ; aucune instruction AVX obligatoire.

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
python scripts/build_installer.py build-sdr-deps/nsis-source/nsis-3.12/makensis.exe 0.1.9
```

Lire [la qualification 0.1.9](docs/QUALIFICATION-0.1.9.md), [les rapports historiques](docs/VALIDATION.md) et [les licences tierces](docs/THIRD_PARTY.md). Distinguer tests automatiques, références indépendantes, captures privées et essais RF. Windows 10, AMD, autres éditions Windows et H24 restent à qualifier ; aucune compatibilité universelle annoncée.

