# POCSAG SDR Pro 0.1.17 — SDR USB et diagnostic Windows

Ajout des adaptateurs natifs Airspy R2/Mini et HF+, avec leurs DLL officielles compilées en x64, et d’un adaptateur expérimental FUNcube HID/audio IQ Windows. RTL-SDR, HackRF et Soapy conservés. Nouvelle fenêtre Récepteur → SDR détectés et pilotes : liste API séparée de l’inventaire USB Windows, actualisation, capacités, essai IQ de cinq secondes, diagnostic DLL/pilote/service et configuration d’un runtime Soapy externe cohérent.

Identités persistantes, fusion native/Soapy par série, refus des séries ambiguës et conservation d’une sélection absente. Aucun pilote installé ou remplacé. Décodeur, deux tableaux, fréquence cliquable, TXT, SQLite et voix alias/P0 conservés.

Le Blog V4 est le matériel physique disponible aux outils pour cette qualification. Airspy/HF+, HackRF, FUNcube et les contrats Soapy ont des essais simulés, distincts des validations physiques. SDRplay demande encore son API constructeur et le module SoapySDRPlay3 compatibles ; Lime/bladeRF/Pluto demandent également leurs modules/SDK. Ceux-ci ne sont pas fournis et aucune compatibilité universelle n’est annoncée. Voir docs/SDR-COMPATIBILITY.md et docs/QUALIFICATION-0.1.17.md pour les preuves et limitations.

ZIP et installateur **non signés** : aucun certificat Code Signing utilisable n’est disponible. Procédure Authenticode, horodatage et vérification préparée dans docs/WINDOWS-SIGNING.md. SHA-256 publiés dans SHA256SUMS.txt. La signature future ne garantit pas une réputation SmartScreen immédiate. Aucune protection Windows désactivée.
