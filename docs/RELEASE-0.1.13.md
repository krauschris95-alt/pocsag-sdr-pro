# POCSAG SDR Pro 0.1.13 — ALPHA automatique et inspection explicite

AUTO sélectionne les textes ALPHA plausibles, avec incertitude conservée dans le diagnostic. Les règles ALPHA/NUMERIC/TONE configurées restent prioritaires. Les charges Base64 en ALPHA/AUTO sont signalées sans modifier le texte ni déclencher une annonce vocale trompeuse. NUMERIC, TXT, voix, WAV/JSON et disposition des tableaux conservés.

Comparaison directe des moteurs 0.1.10, 0.1.11, 0.1.12 et nouveau code sur trois captures : 62 charges utiles complètes inchangées ; les chaînes Base64 existent déjà dans les anciennes versions. Cinq chaînes Base64 binaires sont aussi corroborées par multimon-ng sur une capture 169,626 MHz. Leur format applicatif demeure inconnu.

Double-clic : champs absents expliqués pour les anciens messages et analyse du texte sans remplacement de l'original. Nouveau diagnostic en lecture seule de la connexion réellement ouverte dans Enregistrements → Emplacement des données SQLite.

Limite explicite : l'événement utilisateur signalé n'a pas pu être daté ou retrouvé dans la connexion réelle de sa fenêtre ; la base accessible aux outils ne correspond pas au schéma et à l'historique journalisés. Aucune donnée utilisateur réparée, déplacée ou réinitialisée. Voir docs/QUALIFICATION-0.1.13.md.

Les deux anciens messages signalés sont corroborés dans le TXT général : texte Base64 identique de 116 caractères, 85 octets binaires, reçu à 13:10:53 et 13:11:52 avant la publication de 0.1.12. Le rattachement exact de l’UUID à l’une des deux lignes SQLite reste non confirmé. Dix suites CTest réussies, 57,51 secondes.

Réception RTL réelle pendant cinq minutes : 49 messages dans chaque tableau, 147 lignes TXT exactes dans cinq destinations, trois verrous SQLite repris, aucune file TXT restante ; voix et WAV synthétiques simultanés.

ZIP extrait et démarré sans outils de développement ; installateur vérifié sur 211 fichiers puis désinstallé avec conservation du journal témoin. Binaires compilés localement : GitHub Actions bloqué avant compilation par le budget du compte, run 42102252204, aucun artefact Actions.
