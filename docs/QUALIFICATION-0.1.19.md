# Qualification 0.1.19 — voix française

## Causes constatées et correction

La version 0.1.18 séparait tous les chiffres après le premier P0 valide, y compris les quantités ordinaires. Elle remplaçait aussi les tirets lexicaux par des virgules et ajoutait un silence SAPI de 300 ms au premier point (généralement après l’alias). Ces causes de prononciation et de pauses artificielles sont corrigées. Aucun défaut d’interruption par la file en fonctionnement normal n’a été reproduit : la lecture reste séquentielle et attend sa fin. Les arrêts explicites, coupures de voix, délais de sécurité et erreurs de périphérique restent possibles et signalés.

Le moteur audio partagé `preparedAnnouncement` coupe le préfixe uniquement au premier P0 valide. Les deux chiffres finaux sont prononcés en français (10–99) ; pour 00–09, un second zéro est conservé. Les quantités, téléphones et références restent confiés à la voix française, sans épellation globale. Les heures h/minutes sont explicitées. Les séparateurs typographiques deviennent des virgules sans casser Jean-Jaurès ni les nombres composés. Aucun dictionnaire utilisateur supplémentaire n’est imposé.

SAPI reçoit le texte complet dans un élément XML échappé unique, sans insertion de silence ni découpage au premier point. La voie WAV appelle exactement le même backend et le même préparateur que la voix locale. Le serveur officiel PrivateRadio n’est pas modifié ; les anciens WAV restent intacts. Les nouvelles règles s’appliquent aux nouvelles annonces, sans réécrire les fichiers existants.

## Tests locaux

17/17 suites passent en 104,30 secondes (`qualification-0119/ctest.log`) : décodeur, références indépendantes, réception injectée, capcodes, stockage SQLite, TXT, contrat audio PrivateRadio, spectre fixe et réglages de fréquence. Les tests de parole couvrent les six codes demandés, les frontières de codes invalides, plusieurs P0, l’absence de P0, les quantités, heures, adresses à tiret, identifiants conservés et alias ponctués. Le contrat PrivateRadio vérifie l’égalité du texte parlé local/WAV et la conservation de l’original dans SQLite, TXT, tableaux et JSON.

## Véritable voix Windows

Microsoft Hortense Desktop - French, volume 80, vitesse 0. Huit WAV PCM mono 22 050 Hz / 16 bits, générés sans appel réseau, contrôlés : en-tête, taille complète, durée, énergie non nulle et plus longue plage de faible niveau. Rapport `qualification-0119/audio/sapi-report.json`. Durées : codes environ 7,7–8 s, nombres/adresse 24,94 s, message long 27,41 s. Aucun de ces contrôles de signal ne prouve à lui seul la compréhension ni l’absence d’un mot manquant.

La sortie Windows réelle a également été mesurée par WASAPI : annonce terminée, signal détecté, RMS 0,04598 contre 0 au repos, aucune erreur (`windows-output.json`). Aucun microphone enregistré et aucun réglage système changé.

L’utilisateur confirme après lecture locale de P039 : « Oui, prononciation correcte et phrase complète ». Les exemples nombres/adresse et long sont également fournis pour écoute ; leur validation auditive est consignée séparément lorsqu’elle est disponible. Tous les textes de qualification sont synthétiques. Aucune base, aucun message radio utilisateur ni aucun secret publié.

## Non-régression et distribution

Le code SDR, décodeur, spectre, capcodes, stockage, journaux et transport PrivateRadio est inchangé. Les seules modifications de fenêtre principale sont les libellés de version. Le test vocal est accessible avant Enregistrer ; la sélection de voix, vitesse, volume et sortie conserve son mécanisme existant.

Écoute complémentaire : l’utilisateur était absent pendant la diffusion des exemples nombres/adresse et long. Leur validation auditive humaine reste donc non effectuée ; les WAV sont fournis explicitement pour réécoute. La confirmation P039 reste acquise. Capture de la fenêtre compilée : `qualification-0119/speech-settings.png`, choix de voix/vitesse/volume/sortie conservés et message de test modifiable visible.

## Qualification des paquets et CI

GitHub Actions [42222051538](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/42222051538), commit 162bb05 : compilation, **17/17 suites**, déploiement et artefact réussis en 5 min 41 s. Artefact 13503554109, SHA-256 `2e32ceb4619ab29c01e8af93736bd6fa98e36988b9f4650c1ba89e9acba9e09b`. Journal détaillé : `qualification-0119/github-tests.txt`.

ZIP local : extraction et CRC vérifiés, empreintes des 275 fichiers identiques au dossier de distribution, démarrage réussi sans Qt dans le PATH (système Windows seul). Installateur local : installation, démarrage et désinstallation réussis en dossier isolé, 275 fichiers identiques, TXT témoin préservé (`installer-test.json`). Huit exemples disponibles dans `Exemples-voix-francaise/`. Ces paquets locaux qualifiés ne sont pas déclarés identiques octet par octet à l’artefact CI.

Exécutable : SHA-256 `3022e8df43fa37ecfb4a0af73d00c8afe3f856ccbb868233bd6053e162bd9ec8`.
ZIP : `c7c521f2e6b3c8ed903e9075d52c680cc0678c70735818889a1e9865b4d1a903`.
Installateur : `9b70c2621634c82d25ec273d2c8cb20a196e9410c0f7929b0024b0eb347f4d2a`.

Les ajouts après le commit 162bb05 sont uniquement des preuves et rapports. Aucune modification applicative après les essais. Publication : [v0.1.19](https://github.com/krauschris95-alt/pocsag-sdr-pro/releases/tag/v0.1.19).
