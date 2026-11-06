# Qualification 0.1.16 — fréquence cliquable et annonce à partir de P0

## Portée et conservation

Développement sur la 0.1.15 publiée. Aucun changement des algorithmes démodulation, synchronisation, BCH, interprétation, filtres, deux tableaux, formats TXT ou conservation SQLite. Ajout d’une fenêtre d’accord et d’une projection exclusivement audio. Le pilote RTL relit désormais la fréquence demandée avant de signaler l’accord. Toutes les bases, configurations et destinations d’essai sont temporaires. Aucun texte radio réel, IQ, base ou WAV privé publié.

## Fonctionnement de la fréquence

Carte entière cliquable, curseur main, infobulle exacte et activation Entrée/Espace. Dialogue MHz six décimales, point/virgule, sélection initiale, Entrée/Échap. Les incréments sont calculés en Hz entiers : 1, 10, 100, 1 000, 2 500, 5 000, 6 250, 8 330, 10 000, 12 500, 25 000, 50 000, 100 000 et 1 000 000 Hz, plus entier positif personnalisé. Les flèches n’accordent pas le SDR avant validation. Annuler/croix ne changent pas l’accord. Six boutons, noms en infobulle, liste supplémentaire et éditeur avec ajout, suppression, modification et déplacement ; stockage dans les préférences existantes.

Réaccord du récepteur sélectionné : arrêt automatique de son flux, livraison des événements encore étiquetés avec l’ancien accord, nouvelle chaîne DSP, appel du pilote et vérification de la reprise IQ. RTL : code retour et lecture `rtlsdr_get_center_freq`. Soapy : lecture existante du pilote. HackRF : retour contrôlé de `hackrf_set_freq`. Sur refus, tentative de restauration de l’ancienne configuration ; si elle échoue, arrêt et erreur explicite. La validation de fréquence et d’IQ attend au plus huit secondes par tentative, hors délai de fermeture du pilote. L’application ne propose pas d’annulation au milieu de cette séquence pour éviter un état incohérent.

Le récepteur arrêté reste arrêté, le prochain accord est préparé et sauvegardé. Les configurations supplémentaires mémorisées sont mises à jour pour le même appareil et l’ancien accord ; une nouvelle entrée est créée si nécessaire. Les flèches utilisent un incrément exact sans arrondir la saisie à une grille.

## Préparation audio commune

`src/announcement.h` recherche le premier `\bP0[0-9]{2}\b`, insensible à la casse avec frontières Unicode. La suite intégrale alimente les deux sorties, avec chiffres convertis séparément en mots français. Aucun P0 autonome : corps original inchangé. L’alias est conservé, avec repli neutre « Capcode 0000042 » dans les routes existantes. La pause SAPI de 300 ms reste commune à la voix et à la synthèse WAV. Les tirets séparateurs sont rendus par des pauses de ponctuation et le corps vocal normalisé en minuscules ; seuls les champs audio subissent cette projection.

Exemple synthétique : alias `départ ambulance R-MED1`, texte `APPEL: 50263090555 MED, P002 - AGRESSION - BAGARRE, N5 0`, annonce `départ ambulance R-MED1. P zéro zéro deux, agression, bagarre, N cinq, zéro`.

La même fonction `SpeechService::utterance` prépare la lecture locale et `envelope.utterance` destiné au worker WAV. `text` JSON, UUID, texte SQLite, les deux tableaux, TXT Belgique, TXT sélectionné et profils restent indépendants et complets. La génération des anciens WAV n’est pas relancée. Les quatre combinaisons Voix/Pager restent testées ; aucun serveur officiel ni paramètre réseau modifié.

## Tests automatisés et Windows

- 12/12 suites CTest réussies : premier passage 65,77 s, dernier passage 75,36 s après contrôle des bornes, rapports `qualification-0116/ctest.log` et `qualification-0116/ctest-final.log` : décodeur, vecteurs indépendants, ingestion et affichage, reconnexion injectée, SQLite/reprises, capcodes, TXT, voix, WAV/JSON, présentation et nouveau banc d’accord.
- Test vocal : P002/P004/P017/P026/P039/P069, casse et séparateurs, premier de plusieurs codes et suite conservée, zéros, codes intégrés invalides XP002/P0029/P002A/_P002_/éP002, absence de code, alias neutre, file et déduplication.
- Contrat audio : même chaîne exacte reçue par le backend local et le générateur WAV, alias modifié pris en compte, Voix OFF/Pager ON, absence de voix pour les types exclus, original complet dans SQLite, les deux modèles, TXT et JSON, UUID identique.
- Dialogue : souris sur carte, Espace/Entrée, point/virgule, saisie hors plage rejetée, quatorze pas, 37 Hz personnalisé, six favoris, ajout/modification/suppression/ordre et favori supplémentaire, aperçu sans accord, Échap/croix/Entrée, persistance après réouverture.
- Pilote injecté : véritable appel du code de réaccord via DLL de laboratoire, fréquence demandée/relue identique, refus de 170 MHz, ancienne fréquence et IQ restaurés, historique conservé, configuration d’un autre récepteur arrêté sans modification du principal.
- Captures natives du dialogue dans `qualification-0116/scale100`, `scale125`, `scale150` : Glacier et sombre, Compacte/Normale/Grande. Comparaison visuelle effectuée : champ large, flèches lisibles, six raccourcis à droite et bouton Appliquer accessible. Les facteurs sont des facteurs Qt de l’application, sans changement des paramètres DPI système. La machine ne permet pas de qualifier un écran physique 2560×1440. Les captures QWidget montrent la zone cliente du vrai dialogue Qt, hors cadre natif Windows.

## Réception matérielle et audio réels

`qualification-0116/rtl-retune-live.json` : 93 s, RTL-SDR Blog avec tuner R828D, 169,626 → 169,625 → 169,626 MHz ; deux réaccords 1 538 et 1 557 ms. Respectivement 14 434 304 / 14 417 920 / 14 417 920 octets IQ et 121 / 8 / 46 trames décodeur ; IQ actif à chaque étape. 175 trames ne signifient pas 175 messages acceptés : filtres/diagnostics produisent ici 17 lignes SQLite dans le tableau général. Trois messages synthétiques enregistrés occupent le tableau inférieur ; les autres réceptions ne sont pas enregistrées comme capcodes sélectionnés dans cette base temporaire.

Trois annonces SAPI automatiques et trois WAV PCM valides, même préparation contrôlée, pendant que les IQ continuent et que la fenêtre est réduite. Deux journaux simultanés : Belgique quotidien et permanent sélectionné ; toutes les lignes correspondent aux octets de l’outbox. Historique conservé à travers les deux réaccords. Le pilote émet des avertissements transitoires « PLL not locked » à l’ouverture, puis fournit des IQ et des trames ; aucun verrouillage RF externe calibré n’a été mesuré.

`qualification-0116/windows-audio-p0.json` : annonce exacte P002 via le service vocal, voix française, sortie Windows non muette, 805 440 échantillons agrégés, RMS 0,0452 contre zéro au repos, pic 0,4499 ; signal effectivement rendu détecté. Ceci mesure la sortie numérique Windows, sans microphone et sans prétendre à une reconnaissance acoustique de chaque mot. Le contenu exact est vérifié à l’entrée commune des backends. Les confirmations antérieures d’audibilité physique de l’utilisateur restent distinctes de cette mesure.

## Limites explicites

Architecture de fréquence existante en `quint32` : 1 Hz à 4 294 967 295 Hz ; les plages supérieures de certains matériels, dont HackRF, restent indisponibles. Cette limite technique n’est pas une restriction géographique. Le pilote décide de la plage matérielle à l’ouverture ; appareil arrêté, le réglage est préparé sans ouvrir la clé. Les bibliothèques peuvent arrondir physiquement un oscillateur sans exposer cette précision dans leur valeur relue. Aucun autre SDR physique disponible : HackRF et Soapy ne sont pas déclarés qualifiés matériellement par cet essai.

La réouverture du flux prend environ 1,5 s ici ; les messages transmis pendant cette interruption peuvent ne pas être reçus. L’historique antérieur reste conservé. Une panne totale USB peut empêcher la restauration ; l’état devient alors arrêté avec erreur. Pas de test d’endurance de plusieurs jours ni de nouvelle prétention sur le format de charges binaires. Exécutables Windows non signés.

## Livraison

Code applicatif : `167637b8d0a54f68720471c19f9b04d215de139c`. Le tag final ajoute les résultats de distribution, sans changer ce code. Bibliothèques Qt/RTL/Soapy/HackRF et sources/licences épinglées réutilisées de la distribution 0.1.15 qualifiée ; aucun pilote système installé ou remplacé.

ZIP contrôlé par CRC, extrait dans `build-portable-0116`, 230 fichiers identiques par SHA-256 au staging. Démarrage réussi avec PATH Windows uniquement, puis réception RTL par l’exécutable portable dix secondes à 169,626 MHz : 540 672 octets de spectre reçus, aucune erreur pilote. Aucun message ALPHA/NUMERIC accepté pendant cette courte sonde ; la qualification du décodage et du réaccord repose sur l’essai de 93 secondes ci-dessus. Captures `portable.png` et `installed.png`.

Installateur exécuté dans un dossier isolé du dépôt avec garde contre une installation utilisateur déjà enregistrée : installation, lancement et désinstallation code 0, 230 empreintes de fichiers conformes, TXT témoin conservé. La première commande de sonde portable utilisait `$LASTEXITCODE` sans attendre le processus GUI ; ce contrôle invalide du banc a été remplacé par `Start-Process -Wait -PassThru`, puis tous les contrôles ont réussi. Aucun changement du produit nécessaire.

Empreintes des paquets locaux qualifiés (également dans `qualification-0116/release-assets.json`) :

| Paquet | Octets | SHA-256 |
| --- | ---: | --- |
| `POCSAG-SDR-Pro-0.1.16-windows-x64.zip` | 56359384 | `077d49fa18243f50753d996e833cabe02a1a6d997482e390dc22a4a2f8ff3bc5` |
| `POCSAG-SDR-Pro-0.1.16-windows-x64-setup.exe` | 49900620 | `6c2b99b7394b9f21443a58e386a91653bcc62a23ae2d8c414a624e926242952f` |

L’empreinte de l’exécutable compilé est identique dans le staging, le ZIP extrait et l’installation. Le ZIP exclut les fichiers DB, IQ, RAW, WAV, MP4 et INI ; les seuls exemples textuels sont synthétiques. Les rapports historiques restent inchangés.



## GitHub Actions — contrôle distant

Le premier run [42173427758](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/42173427758) a compilé l’application et réussi les nouveaux tests, mais échoué sur l’ancien test de sélection TXT (`storage_regression_tests.cpp`, ancienne ligne 54). Le banc pilotait `selectFile` puis acceptait immédiatement pendant le chargement asynchrone du modèle de fichiers Qt ; il comparait aussi littéralement les chemins Windows, sans reconnaître un alias court/long du même dossier. Le banc renseigne désormais explicitement le champ de nom avec un chemin absolu et compare les parents canoniques. La fenêtre native et tout le code produit restent inchangés.

Validation locale : trois passages consécutifs réussis (32,08 s), puis contrôle des alias de chemin réussi (12,36 s). Journaux `picker-ci-regression.log` et `picker-path-regression.log`. Le tag reste immuable ; cette correction du banc existe dans les commits suivants de main. Nouvelle compilation distante en cours, résultat consigné après sa fin.
