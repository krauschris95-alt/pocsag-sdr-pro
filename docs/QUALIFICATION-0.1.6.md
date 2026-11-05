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

## Essais matériels après libération du récepteur

L’utilisateur a arrêté la réception de 0.1.5, puis les mêmes binaires 0.1.6 ont été testés :

- **RTL natif, 60 secondes, 169,626 MHz, 240 kéch/s : réussi.** 18 messages texte, voyant IQ vert, Save des capcodes réussi pendant que la réception continue, écriture TXT contrôlée, arrêt et voyant rouge. Rapport `qualification-016/hardware-save-after.log`.
- **SoapySDR + SoapyRTL, 300 secondes, 169,626 MHz, 250 kéch/s : réussi.** Appareil `Generic RTL2832U OEM`, tuner R828D. Capacités interrogées sans erreur, AGC et PPM annoncés, flux CF32 reçu et converti dans le moteur existant. 65 messages texte, 18 644 992 octets d’échantillons adressés au spectre (sous-échantillonnage d’affichage, pas le volume IQ total), voyant vert puis rouge à l’arrêt, aucune erreur remontée au récepteur. Rapport `qualification-016/soapy-after-release.json` et journal associé.
- **Comparaison indépendante du même IQ Soapy réel, capture privée de 60 s à 240 kéch/s : 3/3 textes exacts**, capcode/fonction/débit/type/texte concordants, aucun manquant ou supplémentaire sur cet échantillon. Le discriminateur FM/rééchantillonneur scipy et multimon-ng sont distincts de notre moteur ; rapport de compteurs et SHA256 `soapy-independent-reference.json`. Les trois références audio externes 512/1200/2400 ont aussi été rejouées (1/1 chacune). Les IQ, le PCM intermédiaire, les adresses et les textes réels restent hors Git et hors Release. Ces trois messages ne prouvent pas un taux général de réception, et les 65 messages de l’essai de cinq minutes n’ont pas tous été comparés à un oracle.
- Ressources observées : environ 25,4 Mo de mémoire résidente aux relevés 66 s et 228 s ; CPU cumulé 4,22 s à 228 s sur ce poste. Ces relevés ponctuels ne constituent pas un profilage complet, une preuve d’absence de fuite ou un essai H24.
- Les journaux des deux pilotes contiennent un avertissement `PLL not locked!` à l’ouverture ; le flux et le décodage fonctionnent ensuite. Aucune erreur USB ou interruption signalée pendant les essais décrits.
- Les empreintes de l’application, de rtlsdr.dll, de libSoapySDR.dll et du module sont identiques à celles des fichiers déjà distribués : `tested-payload-hashes.json`. Aucun remplacement des exécutables de la Release n’est nécessaire pour ces résultats.

**Historique de l’accès occupé :** avant l’arrêt par l’utilisateur, les deux DLL voyaient la clé mais retournaient USB -3 ; `soapy-real.json` conserve cet échec initial. Ce blocage est levé pour les essais ci-dessus. Aucune fermeture forcée de l’application utilisateur.

Les essais matériels 0.1.5 et les comparaisons indépendantes antérieures restent conservés dans `qualification-015` et VALIDATION.md ; ils ne sont pas confondus avec ces nouveaux résultats. Deux SDR physiques simultanés, autres modèles et reconnexion par débranchement réel restent non testés.

## Matrice obligatoire des 30 essais

| # | Essai | Résultat / preuve et limite |
|---|---|---|
| 1 | Installation Windows 10 | Non effectué : machine absente. |
| 2 | Installation Windows 11 | Réussie localement par installateur silencieux ; une configuration Pro/Intel. |
| 3 | Démarrage sans SDR utilisable | Réussi : application installée et portable, démarrage hors réception. Débranchement physique non réalisé. |
| 4 | Plusieurs familles SDR | Non effectué matériellement. RTL natif, HackRF natif et adaptateur Soapy compilés ; seul module SoapyRTL inclus. |
| 5 | Sélection matériel | Réussie sur la clé présente, nativement et via SoapyRTL ; autres familles non testées. |
| 6 | Démarrage IQ | 0.1.6 matériel réussi : RTL 60 s et Soapy 300 s, plus captures indépendantes. |
| 7 | Voyant vert | Régression et essais matériels RTL/Soapy réussis. Aucun vert simulé pour lecture IQ. |
| 8 | Arrêt rouge | Régression et arrêts matériels RTL/Soapy réussis en 0.1.6. |
| 9 | Déconnexion USB | Rupture injectée réussie ; pas de débranchement physique cette session. |
| 10 | Reconnexion | Rupture/reprise injectée au même numéro de série réussie. Reconnexion physique en attente. |
| 11 | POCSAG 512 | Référence indépendante ALPHA/NUMERIC réussie. |
| 12 | POCSAG 1200 | Référence indépendante ALPHA/NUMERIC réussie ; IQ réels antérieurs et nouvel IQ Soapy comparé à multimon-ng. |
| 13 | POCSAG 2400 | Référence indépendante ALPHA/NUMERIC réussie. |
| 14 | Débit automatique | Références et tests moteur réussis ; aucune qualification RF exhaustive. |
| 15 | Tous les messages en haut | 70 messages, y compris non enregistrés et capcodes inactifs ; intégration et capture. |
| 16 | Capcodes en bas | 24 messages sélectionnés sur les 70 ; deux défilements indépendants. |
| 17 | Ajout capcode | Régression Save réussie. |
| 18 | Modification capcode | Régression Save réussie. |
| 19 | Save | Régression complète et nouvelle sauvegarde avec réception matérielle active 0.1.6 réussies. |
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
| 30 | Prolongé H24 | Soapy 5 min réussi ; H24 non effectué et non démontré. |

## Compatibilité et fonctions restantes

RTL OEM avec tuner R828D : nouvelle DLL native et voie Soapy testées en 0.1.6 ; V3/V4/E4000 spécifiques non qualifiés. HackRF : bibliothèque distribuée, matériel non disponible. Airspy/HF+, SDRplay, Lime, bladeRF, Pluto, USRP, Funcube : extension conditionnelle par modules compatibles, non distribués et non testés. SoapySDR ne garantit pas la compatibilité universelle.

Limites : maximum fréquence 4294,967295 MHz (stockage 32 bits), Soapy CF32 réduit en CU8, taux IQ 120 k à 20 M sous réserve du pilote/CPU/USB. Jusqu’à huit sources supplémentaires autorisées logiciellement, aucun test équivalent de charge matérielle. Un module natif bloquant peut bloquer l’arrêt. Les paramètres constructeur spécifiques autres que RX/gain/AGC/PPM/taux/bande/antenne demandent encore des extensions. Les réglages d’accord effectués sur une source supplémentaire dans le spectre doivent être remémorisés depuis le dialogue pour persister.

Pas de qualification Windows 10, AMD, Famille/Entreprise/Éducation, anciens PC, multiples résolutions physiques, très longs historiques, H24, coupure secteur ou disque plein. Le diagnostic CPU ne prétend pas qu’AVX matériel est utilisable par l’OS ; traitement portable sans AVX obligatoire.

PrivateRadio : préférences persistées, contrat JSON et règle d’éligibilité unitairement testés. Aucun transport réel, authentification, serveur, reconnexion, file distante, canal vocal distant ni transmission cloud. L’interrupteur général est explicitement une intention préparatoire. L’activation reste une phase ultérieure distincte, après validation autonome.


## Provenance CI

Le [run #19](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/41950968610), commit `9990e6fe84d60b8c145149cbb9baaa39c53effb2`, a échoué **avant démarrage** : « The job was not started because an Actions budget is preventing further use. » Aucun artefact Actions de cette version, aucune compilation CI réussie revendiquée. Les livrables Release sont construits et contrôlés localement. Le workflow contient aussi la compilation Soapy/RTL et la génération de l’installateur pour une future exécution quand le budget sera rétabli. Aucun changement de facturation réalisé.


## Contrôle des paquets finaux

ZIP : intégrité ZIP contrôlée, extraction, puis démarrage natif avec PATH limité à Windows (aucun dossier Qt/MinGW de développement), sortie 0. Installateur final : même PATH limité, installation/démarrage/désinstallation sorties 0, journal utilisateur conservé. Voir `portable-test.json`, `installer-final-test.json` et `packages-sha256.json`. Les noms/chemins absolus des artefacts locaux sont des informations de construction, sans messages radio.

Un premier rejeu de l’installation finale a échoué pendant la forte occupation disque par les deux extractions de test ; après retrait des seules extractions générées, le même installateur a réussi. Cet incident ne constitue pas une validation de reprise après disque plein en réception. Les captures/IQ/configurations utilisateur n’ont pas été supprimées.

L’audit PE trouve uniquement deux dépendances optionnelles non fournies sur les plugins Qt PostgreSQL/Mimer (`LIBPQ.dll`, `MIMAPI64.dll`) : ces pilotes ne sont pas utilisés par l’application, qui utilise exclusivement SQLite. Le démarrage et les tests SQLite réussissent sans eux ; voir `dependencies.json`. Aucun service PostgreSQL/Mimer n’est requis.


## Mise à jour après publication

Ces essais matériels ont été réalisés après publication, lorsque l’utilisateur a libéré le SDR. Ils valident les mêmes fichiers déjà livrés, sans changement de l’application ou du moteur. Seule la sonde de test reçoit les arguments optionnels `--rate`, `--seconds` et `--capture` pour reproduire la capture privée de référence. Les documents inclus dans les archives décrivent l’état au moment de leur construction ; ce rapport et les notes de Release constituent le complément de qualification ultérieur. Les empreintes des téléchargements restent inchangées.
