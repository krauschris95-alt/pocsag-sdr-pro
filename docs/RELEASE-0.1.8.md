# POCSAG SDR Pro 0.1.8 — préversion Windows x64

Interrupteurs vocaux par capcode lisibles, commande Voix ON/OFF dans la barre supérieure, capcodes simplifiés avec paramètres avancés. Deux journaux indépendants : Belgique quotidien et fichier permanent choisi pour les capcodes. Nouvelles lignes : CAPCODE JJ/MM/AAAA HEURE MESSAGE, sans conversion des anciens fichiers.

Deux tableaux compacts conservés, commande Derniers messages, colonnes mémorisées et présentation bleu/noir améliorée. Décodeur POCSAG et filtrage du tableau inférieur inchangés. Reprise SQLite et reçus anti-doublons conservés.

Consulter docs/QUALIFICATION-0.1.8.md dans le code source et le ZIP pour les résultats détaillés et limites. Fermer complètement l’ancienne application avant de lancer celle-ci. Données et TXT existants conservés. Exécutables non signés.

Compilation et tests locaux sous Windows 11. GitHub Actions bloqué avant démarrage par le budget du compte (run 41997547900), aucun artefact Actions pour cette version. Windows 10 et indice composite RF/synchronisation restent à qualifier.

Validation : 8/8 suites, SAPI français réel, Save et voix pendant réception réelle. Essai RTL 23 minutes dont 20 minutes avec les deux journaux : 600 textes, 535/538 lignes TXT exactes, dix verrous injectés, aucune attente finale. Décodeur inchangé, comparaisons indépendantes exactes sur les échantillons testés. Aucun message réel dans les captures publiées.

Limites : qualité affichée = contrôle BCH/parité, pas indice composite RF ; Windows 10, autres matériels et H24 non qualifiés. Navigation Qt au-delà de 2,1 milliards de lignes à qualifier, sans suppression de l’historique SQLite.
