# POCSAG SDR Pro 0.1.11 — voix automatique et WAV PrivateRadio

Préversion Windows x64 à partir de 0.1.10. Fermer l’ancienne application avant d’ouvrir la nouvelle. Données conservées ; exécutables non signés.

- Voix française automatique : alias, pause puis message complet, sortie réouverte et réinitialisation contrôlée. Diagnostic des interrupteurs, périphérique et étapes sans texte privé. Trois annonces automatiques confirmées audibles, fenêtre réduite.
- Pager ON : vrai WAV PCM et manifeste JSON, même avec Voix OFF. Dossier personnalisable, écriture contrôlée, noms uniques, publication atomique.
- File SQLite persistante, reprises, expiration et contrôle des fichiers. TXT/tableaux indépendants, décodeur/SDR conservés.
- Préparation HTTPS testée sur LAB isolé : certificat vérifié, authentification, audio, accusés et déduplication. Aucun serveur officiel contacté/modifié. Relais OFF par défaut ; adaptateur de production non raccordé.

Dix suites réussies ; sortie Windows mesurée, quatre WAV SAPI réels pour trois lectures locales. Rapport et contrat dans docs/QUALIFICATION-0.1.11.md et docs/PRIVATERADIO-AUDIO-CONTRACT.md.

Windows 10, débranchement audio physique et liaison serveur/Android non qualifiés. Serveur et clients doivent être intégrés et testés avant de déclarer PrivateRadio opérationnel.

Qualification RTL : dix minutes puis cinq minutes sur le code final, cinq TXT exacts, trois annonces et trois WAV pendant la réception, reprises SQLite sans attente résiduelle. Tests synthétiques et RF distingués dans le rapport.

Distributions compilées et testées localement ; aucun artefact Actions 0.1.11 confirmé. ZIP, installation, lancement et désinstallation conservatrice validés dans des dossiers isolés. Prévoir suffisamment d’espace disque temporaire pour l’installation et les annonces.
