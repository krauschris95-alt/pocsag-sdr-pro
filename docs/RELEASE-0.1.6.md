# POCSAG SDR Pro 0.1.6 — préversion Windows x64

- Correctif Save conservé et régression testée ; options TXT, affichage bas, voix et préparation Pager indépendantes.
- Architecture SoapySDR 0.8 et module RTL fournis, interrogation des capacités RX ; autres familles nécessitent leurs modules/pilotes et une qualification matérielle.
- Plusieurs sources indépendantes, sélection du spectre, fréquence et source dans les messages/TXT. Essai simultané sur deux fichiers IQ indépendants, pas sur deux appareils physiques.
- Interface PDW et deux tableaux conservés ; ajout de Fréquence et Source optionnelle, accès au fichier TXT par capcode.
- Diagnostic CPU, compilation x86-64 sans AVX obligatoire.
- ZIP portable et installateur par utilisateur, sans environnement de développement requis.

PrivateRadio : préférences et contrat hors ligne seulement, aucun transfert actif.

Qualification détaillée : docs/QUALIFICATION-0.1.6.md. Windows 10/AMD/autres SDR et H24 non qualifiés. La clé RTL présente reste inaccessible pendant cette session (erreur USB -3) : aucune nouvelle réception matérielle réussie annoncée. Les essais réels antérieurs 0.1.5 sont conservés et distingués des tests de cette version.

Paquets compilés localement sur Windows 11 Pro / Intel / Qt 6.8.3 / MinGW 13.1. L’état du job GitHub Actions est documenté séparément ; ne pas assimiler la présence d’un workflow à une compilation CI réussie.
