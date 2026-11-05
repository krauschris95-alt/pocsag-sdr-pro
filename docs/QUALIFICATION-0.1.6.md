# Qualification 0.1.6 — 5 novembre 2026

Préversion développée depuis 0.1.5, sans réécriture du décodeur. Compilation locale Release x64, Qt 6.8.3, GCC 13.1 / MinGW, Windows 11 Professionnel 26H2 (10.0.26300), Intel i7-8550U, 4 cœurs / 8 threads. Aucune autre machine disponible. Aucun résultat de ce rapport ne valide universellement les modèles SDR ou Windows.

## Résultats de cette version

- **Six suites CTest réussies, 16,57 s**, journal `qualification-016/ctest.log` : décodeur, intégration, références indépendantes, file vocale, reconnexion RTL injectée, régression Save.
- Intégration rejouée avec plateforme Qt Windows native : réussie. Captures réelles de widgets, données fabriquées explicitement étiquetées : `qualification-016/captures/compact-messages.png`, `capcodes.png`, `journal.png`. Aucun contenu radio privé.
- Deux fichiers CU8 issus du même signal indépendant multimon-ng, relus simultanément dans deux instances distinctes à fréquences déclarées différentes : deux textes exacts, deux identités/fréquences dans SQLite et TXT. Ce test valide l’isolation logicielle ; **il ne valide pas deux SDR physiques ni deux signaux RF différents**.
- TXT par capcode désactivé puis activé alors que copie générale désactivée ; aucune écriture générale indésirable. Caractères littéraux `%8 %9 %1` préservés. Profils multiples, déduplication, reprise d’erreur, minuit Bruxelles et migration réexécutés.
- Save : ajout/modification, plusieurs sauvegardes, persistance, option Pager indépendante, cellule manquante, doublons, chemins invalides, fichier verrouillé, base en lecture seule. Régression nulle reproduite historiquement avant réparation ; preuve dans `qualification-015`.
- Installateur NSIS par utilisateur : installation silencieuse dans dossier de test du projet, lancement réel (code 0), désinstallation (code 0), exécutable retiré et journal utilisateur conservé. Rapports `installer-test.json` et `uninstaller-test.json`. Pas d’installation de Visual Studio/Python sur le poste destinataire nécessaire. Non signé.
- SAPI réel : Microsoft Hortense Desktop française sélectionnée, lecture terminée sans erreur (`speech-real.json`). La vérification porte sur le moteur Windows ; aucune mesure acoustique ni validation de prononciation du trafic réel.
- Audit d’imports : nouvelle DLL RTL recompilée depuis les mêmes sources Blog V1.4.0, dépend de libusb et DLL système ; retrait de pthreadVC2 et dépendances MSVCR100/VCRUNTIME140. Soapy et module utilisent les runtimes MinGW fournis.

## Essai matériel actuel : non réussi / accès occupé

La version 0.1.5 ouverte depuis Téléchargements utilise la clé. Aucune fermeture forcée de cette application. Les DLL officielle précédente et recompilée voient chacune un appareil mais renvoient `rtlsdr_get_device_usb_strings = -3` (accès). Le module SoapyRTL ne retourne aucun appareil accessible. `soapy-real.json` conserve cet échec ; aucune réception Soapy de 60 s ou 5 min n’a pu démarrer. **Ne pas confondre une sonde compilée avec un essai matériel réussi.** La voie Soapy et la nouvelle compilation native restent à qualifier sur IQ réels dès libération du récepteur.

Les essais matériels réussis 0.1.5 sont antérieurs : RTL OEM 60 s à 169,626 MHz, voyant IQ vert, Save pendant réception, 5 textes, TXT et arrêt rouge. Rapports conservés dans `qualification-015`. Les comparaisons indépendantes du même IQ réel (11 textes exacts acceptés) restent détaillées dans VALIDATION.md, sans nouvel essai RF revendiqué en 0.1.6.

## Matrice obligatoire des 30 essais

| # | Essai | Résultat / preuve et limite |
|---|---|---|
| 1 | Installation Windows 10 | Non effectué : machine absente. |
| 2 | Installation Windows 11 | Réussie localement par installateur silencieux ; une configuration Pro/Intel. |
| 3 | Démarrage sans SDR utilisable | Réussi : application installée et portable, démarrage hors réception. Débranchement physique non réalisé. |
| 4 | Plusieurs familles SDR | Non effectué matériellement. RTL natif, HackRF natif et adaptateur Soapy compilés ; seul module SoapyRTL inclus. |
| 5 | Sélection matériel | Énumération et configuration implémentées ; sélection RTL réelle qualifiée en 0.1.5, nouveau Soapy en attente. |
| 6 | Démarrage IQ | 0.1.5 matériel réussi ; 0.1.6 captures indépendantes réussies, USB occupé. |
| 7 | Voyant vert | Régression avec injection réussie ; essai matériel 0.1.5 réussi. Aucun vert simulé pour lecture IQ. |
| 8 | Arrêt rouge | Régression injection et essais antérieurs réussis. |
| 9 | Déconnexion USB | Rupture injectée réussie ; pas de débranchement physique cette session. |
| 10 | Reconnexion | Rupture/reprise injectée au même numéro de série réussie. Reconnexion physique en attente. |
| 11 | POCSAG 512 | Référence indépendante ALPHA/NUMERIC réussie. |
| 12 | POCSAG 1200 | Référence indépendante ALPHA/NUMERIC réussie ; IQ réels antérieurs. |
| 13 | POCSAG 2400 | Référence indépendante ALPHA/NUMERIC réussie. |
| 14 | Débit automatique | Références et tests moteur réussis ; aucune qualification RF exhaustive. |
| 15 | Tous les messages en haut | 70 messages, y compris non enregistrés et capcodes inactifs ; intégration et capture. |
| 16 | Capcodes en bas | 24 messages sélectionnés sur les 70 ; deux défilements indépendants. |
| 17 | Ajout capcode | Régression Save réussie. |
| 18 | Modification capcode | Régression Save réussie. |
| 19 | Save | Régression complète réussie ; réception matérielle simultanée validée en 0.1.5, non répétée ici. |
| 20 | Persistance capcodes | Fermeture/réouverture et migration réussies. |
| 21 | Dossier TXT | Création et validation chemins réels en tests ; dialogue Windows branché sur getExistingDirectory. Choix manuel non rejoué. |
| 22 | Fichier TXT | Création, ajout, verrou et erreurs testés ; dialogue getSaveFileName. Choix manuel non rejoué. |
| 23 | Plusieurs journaux | Intégration réussie : général/spécifiques/profils, sans doublon par destination. |
| 24 | Voix ON | File/moteur injectable réussis ; sonde SAPI réelle séparée. |
| 25 | Voix OFF | Général et individuel testés sans nouvelle annonce. |
| 26 | Alias avant texte | Chaîne exacte contrôlée dans test ; sonde SAPI annonce d’abord l’alias de test. |
| 27 | Réception avec voix | Injection bloquant la voix pendant nouvelles arrivées : SQLite/TXT/affichage continuent. RF simultané non rejoué. |
| 28 | Redimensionnement | Glissement souris synthétique sur vrai QSplitter, ratio 60/40 initial, restauration et surface >88 % contrôlés. |
| 29 | Spectre réel | Chaîne FFT sur IQ existante conservée ; captures indépendantes et essais RF antérieurs. Aucun faux spectre fourni comme mesure. |
| 30 | Prolongé H24 | Non effectué. Aucun engagement H24 démontré ; essai de 5 min Soapy bloqué avant démarrage. |

## Compatibilité et fonctions restantes

RTL OEM : matériel historiquement testé ; V3/V4/E4000 spécifiques non qualifiés. HackRF : bibliothèque distribuée, matériel non disponible. Airspy/HF+, SDRplay, Lime, bladeRF, Pluto, USRP, Funcube : extension conditionnelle par modules compatibles, non distribués et non testés. SoapySDR ne garantit pas la compatibilité universelle.

Limites : maximum fréquence 4294,967295 MHz (stockage 32 bits), Soapy CF32 réduit en CU8, taux IQ 120 k à 20 M sous réserve du pilote/CPU/USB. Jusqu’à huit sources supplémentaires autorisées logiciellement, aucun test équivalent de charge matérielle. Un module natif bloquant peut bloquer l’arrêt. Les paramètres constructeur spécifiques autres que RX/gain/AGC/PPM/taux/bande/antenne demandent encore des extensions. Les réglages d’accord effectués sur une source supplémentaire dans le spectre doivent être remémorisés depuis le dialogue pour persister.

Pas de qualification Windows 10, AMD, Famille/Entreprise/Éducation, anciens PC, multiples résolutions physiques, très longs historiques, H24, coupure secteur ou disque plein. Le diagnostic CPU ne prétend pas qu’AVX matériel est utilisable par l’OS ; traitement portable sans AVX obligatoire.

PrivateRadio : préférences persistées, contrat JSON et règle d’éligibilité unitairement testés. Aucun transport réel, authentification, serveur, reconnexion, file distante, canal vocal distant ni transmission cloud. L’interrupteur général est explicitement une intention préparatoire. L’activation reste une phase ultérieure distincte, après validation autonome.
