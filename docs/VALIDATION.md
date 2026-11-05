# Validation 0.1.4 — correction prioritaire de disposition

## Agencement comparé à la maquette PDW

La fenêtre principale ne conserve qu'une barre de menus, une petite barre d'outils et les deux onglets. Fréquence cyan en haut à droite. Aucun chemin TXT, grand titre, réglage secondaire ou barre de recherche au-dessus des tableaux. Deux tableaux pleine largeur, séparateur horizontal et partage initial 60/40. La gestion des capcodes reste dans son menu ; aucun troisième tableau permanent n'est ajouté.

Tests Qt ajoutés : rapport initial du panneau supérieur entre 57 et 63 %, hauteur cumulée des tableaux supérieure à 88 % de la zone centrale ; panneau inférieur supérieur à 250 pixels dans la fenêtre testée. Déplacement réel du handle par événements souris de 100 pixels, changement des deux hauteurs puis restauration de leur proportion après destruction/recréation de la fenêtre. Deux défilements indépendants vérifiés avec 70 messages, dont 24 appartenant au seul capcode actif sélectionné ; un capcode inactif et un non enregistré restent uniquement en haut. Colonnes 7/8 vérifiées.

Captures Windows natives : interface réelle avec textes synthétiques explicitement marqués « Message de validation interface », pour ne pas publier les transmissions reçues. Vérification visuelle : tableaux pleine largeur, absence de barres encombrantes, couleurs PDW et second onglet exclusivement dédié au spectre/waterfall.

Les cinq suites de non-régression couvrent toujours le décodage indépendant, SQLite, les fichiers TXT/routages/minuit belge, les capcodes, la voix et la reconnexion injectée. Aucun changement dans decoder.cpp, dsp.h, receiver.cpp, journalworker.cpp ou speech.cpp pour cette correction. Les limites matérielles et d'endurance ci-dessous restent applicables.

Compilation locale Windows x64 Release réussie. CTest : 5/5, zéro échec, 11,56 secondes. L'intégration a également été exécutée avec le plugin Windows natif : code de sortie 0. Captures : interface-0.1.4.png et spectrum-0.1.4.png. Les éventuels avertissements de nettoyage de répertoires temporaires de tests ne concernent pas la base utilisateur.

Paquet 0.1.4 : nouvel essai RTL réel de 15 secondes à 169626000 Hz / 240000 éch/s, PATH limité à Windows, code de sortie 0, deux textes décodés et aucune erreur signalée. Compteurs uniquement dans portable-validation-0.1.4.json, aucun texte radio privé conservé dans le dépôt.

# Validation 0.1.3 — cahier des charges officiel

## Compilation et automatisation

Windows x64 Release, GCC 13.1, Qt 6.8.3 : cinq suites CTest réussies (moteur, intégration Qt/SQLite/TXT, références indépendantes, file vocale, reconnexion injectée). Le pilote de test reste hors du paquet livré. Les résultats matériels ci-dessous ne sont pas déduits des doubles de test.

Dernier passage des cinq suites : 13,55 secondes, zéro échec. GitHub Actions [run 41934951697](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/41934951697) est bloqué avant démarrage : « The job was not started because an Actions budget is preventing further use. » Aucun succès CI 0.1.3 revendiqué ; le ZIP est compilé et testé localement.

Paquet portable : ZIP extrait dans un dossier distinct ; lancement graphique réussi et nouvelle réception réelle de 15 secondes avec PATH limité à Windows, sans Qt/MinGW de développement. Code de sortie 0, 3 messages texte et aucune erreur USB. Le paquet ne contient ni DLL de test, ni base utilisateur, ni capture IQ privée. Ce contrôle est effectué sur le PC Windows de développement, pas sur une seconde machine vierge.

| Exigence | Preuve 0.1.3 | Limite |
|---|---|---|
| Deux tableaux et deux onglets | Tests Qt : tous les textes en haut, sélection active en bas, colonnes et onglets | Contrôle visuel séparé |
| Voix individuelle et générale | Backend bloqué de test : alias avant texte, une seule lecture malgré deux tableaux, FIFO, limite et arrêt | Ne prouve pas toutes les cartes audio |
| Réception pendant voix | Insertion SQLite et TXT continuent pendant blocage volontaire de la voix | Test logiciel, pas endurance matérielle |
| Voix Windows hors ligne | SAPI natif terminé sans erreur, Hortense Desktop française | Rapport speech-validation-0.1.3.json |
| RTL réel | 60 secondes, 169626000 Hz, 240000 éch/s, quatre messages texte, aucune erreur USB | Rapport radio-validation-0.1.3.json ; aucun texte privé publié |
| Reconnexion | DLL de test : première ouverture interrompue, réouverture du même identifiant et reprise du décodage | Débranchement physique non réalisé |
| Journaux | File SQLite persistante, reprise après erreur, TXT UTF-8, routage multiple, minuit Bruxelles et DST | Coupure électrique en cours d'écriture non testée |
| Filtres | ALPHA/NUMERIC masqués restent enregistrés ; capcodes sélectionnés indépendants | TONE/diagnostics sont des refus d'ingestion explicites |
| Paramètres et migration | Base migrée, paramètres capcodes/voix, filtres, dossiers et pas persistants | Migration sur copie de test |
| HackRF | Backend compilé, DLL et dépendances distribuées, API et plages contrôlées | Aucun HackRF connecté : non validé matériellement |
| Stabilité H24 | Workers séparés, watchdog et journal technique borné | Aucun essai 24 h/7 jours : non qualifié |

La fréquence affichée est confirmée par l'événement de réglage matériel réussi. Un changement d'accord redémarre la chaîne, avec interruption possible. Le décodage des trois débits et les références indépendantes ci-dessous restent couverts. Aucun pourcentage de qualité ou SNR artificiel n'est affiché.

## Validation du décodeur héritée de 0.1.2

# Validation 0.1.2 — issue #1

## Résultats locaux

Compilation Windows Release x64 : GCC 13.1 / Qt 6.8.3. Trois suites CTest passent : moteur, intégration Qt/SQLite/journaux, références IQ indépendantes. Les jobs GitHub Actions 0.1.2 sont bloqués avant démarrage par le budget du compte. Aucun résultat CI de cette version : ZIP construit et testé localement.

## Tests synthétiques internes

BCH : mot idle connu, 528 masques d'un/deux bits, trois erreurs rejetées. Trois débits, deux polarités, quatre fonctions, RIC maximal, messages multibatches, sync corrigée, perte/reprise de sync. Les messages partiels à la fin de capture sont abandonnés.

ALPHA avec fonction 0 est interprété correctement en mode ALPHA : plus de convention universelle par fonction. AUTO signale les ambiguïtés. Tests IQ FSK internes pour les trois débits et polarités, numérique explicite.

## Références synthétiques indépendantes : 6/6

`tests/fixtures` contient six IQ produits par **gen-ng de multimon-ng**, pas par notre encodeur : trois débits × ALPHA/NUMERIC, avec les symboles numériques `. U - ] [`. Résultats vérifiés par le décodeur externe avant fixation du manifeste. CTest les démodule et vérifie capcode, fonction, débit, type et texte.

Outil : multimon-ng commit `b5f7d71c5e9b5a76093ba55c515968dc40f92804`. Pour sa compilation MinGW, C est activé en plus de CXX dans son CMake Windows, sans modification des algorithmes. Cet outil ne fait pas partie du logiciel livré. Reproduction : `scripts/make_reference_fixtures.py`.

## Enregistrements externes : 3/3

Fichiers `test/samples/POCSAG_sample_-_{512,1200,2400}_bps.flac` du dépôt multimon-ng. Un message attendu par fichier : tous les champs et textes concordent, sans sortie supplémentaire. Ce sont des enregistrements **audio externes**, remodulés en CU8 à ±4500 Hz pour tester notre entrée IQ, et non des captures IQ radio originales. SHA256 et compteurs dans `reference-validation-30s.json`.

## Captures IQ réelles privées : même transmission comparée

RTL2832U OEM, 169626000 Hz, 240000 éch/s, gain supporté le plus proche de 42 dB. Captures CU8 de 30 et 60 secondes sans erreur USB signalée. Notre moteur relit les CU8 exacts. Pour multimon-ng, un discriminateur FM et un rééchantillonneur scipy indépendants produisent du PCM 22050 Hz depuis ces mêmes CU8. Les deux outils sont explicitement en ALPHA : le type n'est pas prétendu déduit du protocole.

| Capture | Candidats de référence | Textes acceptés ici | Concordance exacte | Autres résultats |
|---|---:|---:|---:|---|
| 30 s | 1 | 1 | 1/1 affiché | aucun supplémentaire |
| 60 s | 11 | 10 | 10/10 affichés | 1 contenu non textuel diagnostique |

Le douzième candidat sur l'ensemble contient GS/ETB/ESC/BS. La charge utile correspond aussi à la référence après conversion symétrique de la représentation des contrôles ; elle n'est pas déclarée texte exploitable. **11 textes acceptés sur 12 candidats : 100 % des textes acceptés concordent, 91,7 % des candidats sont affichables par défaut.** La nature du contenu à contrôles n'est pas identifiée ; aucune conclusion de chiffrement.

Normalisation : la référence rend les contrôles en balises. Les terminateurs NUL/ETX/EOT et espaces de remplissage de fin sont exclus symétriquement. Ce taux ne compare pas les décorations d'affichage. Rapports : `reference-validation-30s.json`, `reference-validation-60s.json`. Reproduction : `scripts/compare_reference.py`.

Les rapports ne contiennent que compteurs et empreintes. Captures, adresses et textes réels restent hors Git. Aucune comparaison PDW réalisée. Cet échantillon limité ne permet pas d'annoncer un taux général de réception ou une fidélité universelle.

## Tests d'intégration et ergonomie

- TONE désactivé avant SQLite, les deux panneaux et TXT ; activation volontaire, masquage des anciens TONE, persistance du filtre, annulation des écritures en attente de types désactivés. Le compteur texte exclut TONE et diagnostics.
- Journaux UTF-8 en ajout, général + spécifique, déduplication des destinations, reprise d'erreur, absence de réécriture après remise en attente, changement de fichier.
- 23:59:59 puis 00:00:01 à Bruxelles : chemins datés corrects. Racine bloquée pendant les réceptions puis débloquée : les reprises conservent leurs destinations d'origine.
- Création automatique des dossiers ; deux capcodes avec deux dossiers/fichiers quotidiens distincts. Fuseau Europe/Brussels vérifié en hiver et en été.
- Migration depuis le schéma capcodes/profils 0.1.1 : alias, chemins, profils conservés et colonnes initialisées. Relecture de l'historique et contrôle d'intégrité SQLite.
- Tous les pas prévus testés par boutons et clavier ; molette testée. Le pas personnalisé passe par le même mécanisme et est persisté.
- Ligne rouge vérifiée au rendu jusqu'au bas du waterfall ; zoom centré réel. Historique de 305 messages et affichage des nouveaux en tête.

## Limites restantes

- Les V3/V4 spécifiques, les niveaux de bruit/décalages/brouillages et conventions de codage ne sont pas systématiquement qualifiés. ASCII 7 bits supporté, pas tous les jeux nationaux. Aucun déchiffrement promis.
- Sync initial exact, sync de batch tolérant deux bits. Filtre FIR, décimation, discriminateur et horloge qualifiés seulement sur les jeux décrits. Un défaut non corrigible produit un rejet.
- Un message interrompu à l'arrêt ou en fin de capture est abandonné. Plafond de sécurité : 65520 bits utiles ; dépassement rejeté.
- AUTO est conservateur et peut demander un choix manuel. Le changement d'accord redémarre brièvement le SDR ; aucune continuité garantie pendant ce réaccord.
- Conservation SQLite manuelle avec confirmation ; aucune suppression des TXT. Pas de qualification 24 h/24, coupures électriques, disque plein, stockage réseau ou modifications externes concurrentes prolongées. La reprise cherche la ligne dans la fin du fichier.
- Pas de HackRF, multicanal, reconnexion USB automatique ou démarrage Windows automatique. Rafraîchissement des grands historiques à optimiser.
- Sauvegarde/restauration par interface, sélection à la souris et déplacement dans le spectre restent hors de cette version. Aucun SNR artificiel.

