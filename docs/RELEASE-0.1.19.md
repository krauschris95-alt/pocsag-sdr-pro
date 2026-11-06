# POCSAG SDR Pro 0.1.19 — voix française et codes P0

Correction de la préparation audio commune à Windows et aux WAV PrivateRadio : P002/P004 conservent les deux zéros, P017/P026/P039/P069 deviennent « P zéro dix-sept / vingt-six / trente-neuf / soixante-neuf ». Les autres nombres ne sont plus systématiquement épelés ; 18h30 devient « dix-huit heures trente ». Les tirets dans les mots et adresses sont conservés, les séparateurs d’informations deviennent des virgules.

Suppression du silence SAPI forcé au premier point. Une annonce complète est soumise en une seule lecture ; les pauses sont confiées à la ponctuation de la voix française. Les réglages de voix, sortie, volume et vitesse sont conservés. Outils → Synthèse vocale propose un message de test modifiable à écouter avant validation.

Les textes reçus restent inchangés dans les tableaux, SQLite, TXT et JSON. Aucun changement du décodeur, de la réception, du spectre fixe −20 / −100 dBFS, du waterfall ou du serveur PrivateRadio.

Qualification : 17/17 tests locaux, véritables WAV produits par Microsoft Hortense Desktop, mesure de sortie Windows réussie et confirmation humaine de P039 (« trente-neuf », phrase complète). Huit exemples audio synthétiques sont joints dans les paquets. Voir docs/QUALIFICATION-0.1.19.md pour les limites et résultats détaillés.

ZIP et installateur Windows x64 non signés ; empreintes SHA-256 jointes.
