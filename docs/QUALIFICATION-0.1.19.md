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
