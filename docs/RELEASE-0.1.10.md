# POCSAG SDR Pro 0.1.10 — fichiers par capcode, profils TXT et voix

Préversion Windows x64. Téléchargez le ZIP portable ou l’installateur, fermez l’ancienne application puis ouvrez la nouvelle. Les données existantes restent conservées.

- Fichier TXT permanent par capcode, Parcourir/Ouvrir, plusieurs capcodes par fichier.
- Profils nommés avec sélection multiple recherchable des capcodes, plusieurs destinations par capcode et déduplication des chemins identiques.
- Journal Belgique quotidien indépendant ; anciens TXT conservés, ajout UTF-8 au format CAPCODE DATE HEURE MESSAGE.
- Voix française autorisée par défaut, alias puis texte complet ; coupure générale explicite mémorisée. Audibilité confirmée sur le PC d’essai.
- Tableau inférieur indépendant des filtres généraux et compatible avec les anciens capcodes texte à zéros initiaux ; effacement direct de son filtre propre.
- Historique SQLite sans purge, navigation progressive à offsets 64 bits et mémoire bornée.
- Migration des données activée uniquement par l’utilisateur, avec sauvegarde, copies vérifiées et original conservé.
- Reprise des destinations TXT bloquées sans empêcher les autres destinations de progresser.

Décodeur POCSAG et démodulation conservés ; aucun relais réseau PrivateRadio ajouté. Les preuves ne contiennent que des données synthétiques et des mesures agrégées.

Compilation locale Windows 11 / Qt 6.8.3 / MinGW 13.1 x64 : le workflow Actions du commit a été refusé avant démarrage par le budget du compte (run 42031451417). Aucun artefact Actions n’a été produit. Exécutables non signés ; Windows 10, clé USB physique et partage réseau autorisé non qualifiés.

Voir le rapport QUALIFICATION-0.1.10.md et les résultats de qualification dans le dépôt.

Validation : neuf suites réussies ; vingt minutes de réception multi-fichiers (448 textes / 1 344 lignes) et contrôle final de cinq minutes réussi après correction du moniteur de test. Reprises SQLite, cinq destinations exactes, deux tableaux et voix simultanés. Voir le rapport pour les résultats initiaux et leurs limites.
