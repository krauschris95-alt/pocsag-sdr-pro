# POCSAG SDR Pro 0.1.3 — préversion Windows x64

Évolution du logiciel existant, sans SDRSharp, PDW ni câble audio virtuel.

- Deux onglets : Messages et Spectre / Waterfall. Deux tableaux superposés : tous les textes décodés en haut, uniquement les capcodes actifs sélectionnés en bas. Colonnes POCSAG, type, vitesse, date, heure et message complet ; alias supplémentaire en bas. Séparateur et dimensions mémorisés.
- Synthèse vocale Windows SAPI hors ligne : interrupteur général et par capcode, alias puis texte, voix française préférée, sortie audio, volume, vitesse, test, relecture et arrêt. File FIFO séparée avec limite et politique de débordement configurables.
- Réception RTL asynchrone, surveillance d'absence d'IQ et reconnexion au même numéro de série. Paramètres matériels et DSP appliqués ; backend HackRF et bibliothèques x64 intégrés.
- Journaux TXT dans un traitement séparé avec file SQLite persistante, reprise sur erreur, UTF-8, classement quotidien belge et dossiers indépendants par capcode.
- Recherche historique par capcode, alias, texte, fréquence, date, débit et type. Les filtres ALPHA/NUMERIC concernent l'affichage et conservent le journal brut ; TONE ONLY reste exclu par défaut conformément au choix utilisateur.
- Spectre FFT réel, waterfall, repère rouge, bande du canal, axes MHz, zoom, échelle verticale, contraste, cadence et accord par clic. Pas personnalisable conservé.
- Démarrage automatique de réception et lancement Windows facultatifs, désactivés initialement ; journal technique local sans texte radio.

Validation : cinq suites CTest réussies ; réception RTL réelle 60 secondes à 169,626000 MHz, quatre messages texte décodés sans erreur USB ; test vocal natif réussi avec Microsoft Hortense Desktop. Les références indépendantes du décodeur sont conservées.

GitHub Actions est bloqué avant démarrage par le budget du compte (run 41934951697). ZIP construit et testé localement sous Windows x64, sans succès CI revendiqué.

Limites : pas de qualification H24/7 jours, pas de test physique HackRF/V3/V4 ni de débranchement USB manuel. La reconnexion a été testée par injection de panne dans une DLL de test absente du paquet livré. Les changements de paramètres redémarrent la réception et peuvent perdre des messages durant l'accord. Canal unique. Voir VALIDATION.md pour les preuves et limites.
