# POCSAG SDR Pro 0.1.12 — Interprétation numérique et diagnostic conservé

NUMERIC intégré aux politiques par capcode, TXT, voix chiffre par chiffre et WAV/JSON. ALPHA/NUMERIC/TONE/AUTO persistants ; AUTO conserve l'ambiguïté et n'annonce pas des données incertaines. Double-clic : candidats ALPHA/NUMERIC, bits, mots radio et diagnostic. Préfixes incomplets conservés. Migration SQLite additive, anciens TXT et historique préservés. Deux tableaux et moteur de démodulation conservés.

Six références IQ indépendantes concordent exactement avec multimon-ng : ALPHA et NUMERIC à 512/1200/2400, y compris symboles étendus. Capture RTL réelle de 60 secondes : 49 correspondances exactes dans chaque représentation et un message supplémentaire non corroboré. Le format applicatif des données compactes reste inconnu ; aucun émetteur NUMERIC réel identifié n'est revendiqué.

Dix suites CTest réussies. Réception RTL pendant cinq minutes : 109 messages dans chaque tableau, 327 lignes TXT exactes dans cinq destinations, trois verrous SQLite repris, aucune file TXT restante ; SAPI et WAV synthétiques simultanés.

Voir docs/QUALIFICATION-0.1.12.md et les preuves jointes pour les tests, l'essai prolongé et les limites. Anciennes préférences conservées : sélectionner AUTO explicitement pour remplacer une ancienne politique ALPHA. Pour un capcode dont le format est connu, sélectionner ALPHA ou NUMERIC dans ses paramètres avancés.

ZIP portable et installateur Windows x64 compilés localement. Source disponible au tag. Aucun texte radio privé, IQ ou audio utilisateur publié. Aucun serveur officiel PrivateRadio contacté.
