# POCSAG SDR Pro 0.1.16 — fréquence cliquable et voix personnalisée

L’encadré de fréquence ouvre directement la fenêtre de réglage : six décimales, point/virgule, quatorze pas et pas personnalisé, six raccourcis et favoris modifiables/mémorisés. Appliquer vérifie le pilote et le retour des IQ ; restauration de l’ancien accord sur refus, sans redémarrage manuel.

Voix locale et nouveaux WAV : alias personnalisé, pause, premier code autonome P0xx puis toute la suite, avec chiffres séparés en français. Sans P0 : fonctionnement précédent. Textes originaux SQLite/TXT/tableaux/JSON et anciens WAV conservés. Thème Bleu Glacier et sombre, deux tableaux et interrupteurs indépendants conservés. Aucun serveur officiel contacté.

Validation : douze suites CTest réussies ; RTL-SDR réel pendant 93 s, deux réaccords en 1,54/1,56 s, 43 270 144 octets IQ, 175 trames du décodeur, 17 messages SQLite, trois annonces SAPI et trois WAV valides pendant la réception réduite. Deux TXT simultanés exacts. Signal vocal réellement mesuré à la sortie Windows. Fenêtre testée en clair/sombre, trois tailles de texte et facteurs Qt 100/125/150 %.

Limites : architecture actuelle de fréquence sur 32 bits, plafond 4 294,967295 MHz ; plage effectivement acceptée vérifiée par le pilote lors de l’ouverture. Un récepteur arrêté prépare seulement l’accord, validé matériellement au démarrage. HackRF/Soapy réutilisent leurs interfaces existantes mais n’ont pas été éprouvés sur du matériel disponible ici. Réouverture automatique du flux au réaccord ; elle réinitialise le DSP. Pas de qualification sur écran physique 2560×1440 ni de plusieurs DPI système. Exécutables non signés.

Voir `docs/QUALIFICATION-0.1.16.md` pour les preuves et les limites détaillées. ZIP portable, installateur par utilisateur et sources disponibles dans cette préversion. Fermer l’ancienne application avant d’utiliser le RTL-SDR dans la nouvelle.

ZIP portable et installateur vérifiés : lancement avec PATH système seul, 230 fichiers identiques par empreinte, réception RTL du ZIP confirmée, désinstallation conservant un TXT témoin. Code applicatif `167637b8d0a54f68720471c19f9b04d215de139c` ; le tag ajoute les rapports de distribution. Les empreintes SHA-256 des téléchargements sont dans `docs/qualification-0116/release-assets.json`.
