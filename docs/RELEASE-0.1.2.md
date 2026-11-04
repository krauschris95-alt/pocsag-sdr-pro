# POCSAG SDR Pro 0.1.2 — préversion

Correctifs et fonctions de l'issue #1 : interprétation ALPHA/NUMERIC explicite globale
et par capcode, TONE ONLY désactivé avant stockage/journalisation, menus de réception
et filtres persistants, journaux quotidiens Europe/Brussels, dossiers par capcode,
repère rouge spectre/waterfall et pas de fréquence personnalisable.

Trois suites de tests : moteur, intégration Qt/SQLite/journaux et vecteurs IQ indépendants.
Comparaison multimon-ng sur trois enregistrements externes et deux captures privées
du récepteur réel. Sur ces dernières : 11 messages texte restitués identiquement ;
un douzième contenu avec contrôles non textuels est conservé comme diagnostic,
désactivé par défaut. L'échantillon ne permet pas de garantir un taux de réception général.

Extraire le ZIP puis lancer POCSAG-SDR-Pro.exe. Bibliothèques Qt/RTL-SDR incluses.
Les bases et chemins existants sont migrés sans suppression.
Pour les journaux quotidiens : activer Enregistrement TXT, choisir la racine et laisser
le profil Belgique ; la structure sera racine/Belgique/AAAA-MM-JJ/Belgique-AAAA-MM-JJ.txt.

Limites : AUTO reste volontairement conservateur ; choisir le mode du réseau/capcode.
Pas de promesse de déchiffrement ou récupération de signaux dégradés. HackRF, multicanal,
reconnexion USB automatique et qualification 24 h/24 restent hors de cette version.
Voir docs/VALIDATION.md pour les résultats exacts et les normalisations de comparaison.

Publication : le budget GitHub Actions du compte bloque les jobs avant démarrage. Ce ZIP est construit et validé localement ; la CI 0.1.2 ne peut pas être exécutée. La facturation reste inchangée.
