# POCSAG SDR Pro 0.1.4 — correction de disposition

L'interface Messages consacre sa zone principale à deux tableaux superposés pleine largeur. Partage initial 60/40, séparateur horizontal déplaçable et position mémorisée. L'ancienne position 0.1.3 est remplacée une seule fois par ce nouveau partage.

Une barre de menus et une petite barre d'outils à icônes, avec la fréquence en gros cyan à droite. Les chemins TXT et réglages secondaires ne sont plus visibles en permanence. Configuration quotidienne Belgique et profils dans Enregistrements ; réception, gain et PPM dans Récepteur ; interprétation et filtres dans Outils ; zoom et réglages du spectre dans Affichage.

Les tableaux ont un fond noir, un texte cyan, dates/heures violettes et mode/vitesse jaunes, des lignes compactes, des colonnes redimensionnables et des défilements indépendants. Double-clic : texte complet. Clic droit : recherche, copie, export et relecture vocale.

Le spectre et le waterfall occupent leur propre onglet. Aucun changement du décodeur, du DSP, du pilote SDR, du worker TXT ou du service vocal. La gestion des capcodes, le TTS individuel, les journaux quotidiens et l'historique restent disponibles.

Voir VALIDATION.md pour les essais de disposition et de non-régression ainsi que les limites de qualification matérielle héritées de 0.1.3.

Validation locale Windows x64 : cinq suites CTest réussies (11,56 s), intégration Windows native réussie, réception RTL réelle de 15 secondes à 169,626000 MHz (deux textes, aucune erreur). ZIP extrait et démarrage sans Qt/MinGW de développement réussis. GitHub Actions reste bloqué avant démarrage par le budget du compte (run 41938307011) ; le ZIP est compilé et testé localement.
