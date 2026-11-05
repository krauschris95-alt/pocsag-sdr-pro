# POCSAG SDR Pro 0.1.6 — préversion Windows x64

- Correctif Save conservé et régression testée ; options TXT, affichage bas, voix et préparation Pager indépendantes.
- Architecture SoapySDR 0.8 et module RTL fournis, interrogation des capacités RX ; autres familles nécessitent leurs modules/pilotes et une qualification matérielle.
- Plusieurs sources indépendantes, sélection du spectre, fréquence et source dans les messages/TXT. Essai simultané sur deux fichiers IQ indépendants, pas sur deux appareils physiques.
- Interface PDW et deux tableaux conservés ; ajout de Fréquence et Source optionnelle, accès au fichier TXT par capcode.
- Diagnostic CPU, compilation x86-64 sans AVX obligatoire.
- ZIP portable et installateur par utilisateur, sans environnement de développement requis.

PrivateRadio : préférences et contrat hors ligne seulement, aucun transfert actif.

Qualification détaillée : docs/QUALIFICATION-0.1.6.md. Windows 10/AMD/autres SDR et H24 non qualifiés. Après libération de la clé : RTL natif 60 s réussi (18 textes, Save pendant réception, TXT, voyant vert/rouge), puis SoapyRTL 5 min réussi (65 textes, aucune erreur remontée). Capture Soapy privée supplémentaire : 3/3 textes exacts face à multimon-ng, sans manquant ni supplémentaire sur cet échantillon. Les mêmes binaires distribués ont été contrôlés par SHA256. Deux SDR physiques et H24 restent non testés.

Paquets compilés localement sur Windows 11 Pro / Intel / Qt 6.8.3 / MinGW 13.1. Le run Actions #19 est bloqué avant démarrage par le budget du compte : aucun artefact Actions disponible pour cette version. ZIP et installateur construits localement ; tests et provenances dans le rapport.
