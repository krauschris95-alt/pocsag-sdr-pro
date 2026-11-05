# Qualification POCSAG SDR Pro 0.1.12

Base : tag v0.1.11, commit 79fd5d29899e96dc58b4a8421f3b1f21d29f5769. Migration SQLite additive v9. Aucune donnée utilisateur modifiée pendant les essais ; bases, TXT et annonces synthétiques de test isolés. IQ radio exclusivement privés sous .tools, jamais dans les livrables.

## Audit et correction

L'extraction des 20 bits utiles, leur ordre reçu, la construction des caractères ALPHA LSB d'abord et des nibbles NUMERIC LSB d'abord correspondent aux vecteurs indépendants. La table numérique existante est correcte : 0123456789.U espace - ][. Sa représentation inversée dans multimon-ng est 084 espace 2.6]195-3U7[. Aucun remplacement artificiel de caractères ou changement non justifié de la démodulation.

Causes logicielles établies : politique ALPHA par défaut, AUTO qui ne conservait que des diagnostics ensuite éliminés par défaut, absence de persistance des candidats/bits dans messages, abandon silencieux des préfixes après erreurs/fin de flux. Il n'est pas établi que tous les textes illisibles rapportés proviennent de ces causes.

AUTO produit AMBIGUOUS en l'absence de règle explicite ; même un alphabet numérique valide ne prouve pas le format de l'émetteur. Politique réseau globale et surcharge par capcode ALPHA/NUMERIC/TONE constituent les règles configurables. Les anciennes préférences sont conservées ; une nouvelle configuration utilise AUTO. Les fonctions 0 à 3 sont conservées sans correspondance arbitraire avec les types. Aucune règle médicale/réseau n'a été inventée pour R-MED1, R-MED2 ou CIP112.

SQLite conserve les bits utiles après BCH, les mots radio hexadécimaux avant correction (adresse, données, mot de limite), les deux candidats, la politique, l'état complet et la cause d'interruption, en plus de fréquence/source/heure/fonction/corrections/UUID. Une trame dont un mot est non corrigeable conserve uniquement son préfixe décodable, marqué INCOMPLETE ; le mot radio fautif reste dans les métadonnées, il n'est pas converti en faux texte. Limite de sécurité d'assemblage 65 520 bits par message, signalée INCOMPLETE, indépendante de la conservation de l'historique.

Double-clic : inspection complète, y compris mots/bits et candidats. Aucune réinterprétation destructive des anciens messages et aucun effacement des anciens TXT. Disposition, ordre chronologique, séparateur et filtres des capcodes conservés. TONE reste désactivé par défaut.

## Tests indépendants

Six fixtures IQ, produites par gen-ng de multimon-ng b5f7d71c5e9b5a76093ba55c515968dc40f92804 et jamais par notre encodeur : ALPHA et NUMERIC à 512/1200/2400. Six comparaisons exactes, dont texte numérique 123-456.789U][. Provenance et script de génération : tests/fixtures/README.md et scripts/make_reference_fixtures.py.

scripts/qualify_numeric.py reproduit les comparaisons depuis les fichiers audio et IQ conservés localement. Le chemin de démodulation de référence est indépendant : filtre rectangulaire 5 points, discriminateur FM complexe puis interpolation 48 000 → 22 050 éch/s, multimon-ng forcé dans chacun des deux modes. Rapport agrégé : qualification-0112/independent-comparison.json.

Capture RTL native 169,626 MHz, 240 000 éch/s, gain demandé 42 dB, 28 800 000 octets, 60 secondes. 484 messages complets dont 434 sans charge utile, 50 avec charge utile ; un préfixe incomplet. Aucune correction BCH sur ces messages. Multimon-ng restitue 49 textes ; les 49 concordent exactement en ALPHA et dans leur représentation NUMERIC. Cinq textes ALPHA compacts d'au moins 24 caractères sans espace et un texte à contrôles/symboles sont également corroborés. Un message supplémentaire 2400 bauds, fonction 0, longueur ALPHA 61 / NUMERIC 110, n'est pas corroboré. La raison de cet écart n'est pas établie et n'est pas masquée.

La concordance indique que les suites compactes comparées existent dans les données reçues ; elle ne permet pas d'identifier un protocole applicatif, un chiffrement ou un contenu voulu. Aucun émetteur réel NUMERIC identifié n'a été confirmé ; les preuves numériques identifiées sont les IQ synthétiques indépendants. Une représentation NUMERIC du trafic belge n'est pas une identification de son type.

## Non-régression et intégration

Tests unitaires : BCH 528 masques à un/deux bits et rejet d'erreurs non corrigeables ; polarités ; ALPHA/TONE aux trois débits ; NUMERIC CU8 aux trois débits ; tous les symboles numériques, toutes les fonctions à chaque débit, fragmentation sur frontière de batch et deux erreurs corrigées ; données à contrôles non textuels ; ambiguïtés et interruption non transformée en NUMERIC.

Test d'intégration : véritable fixture IQ NUMERIC indépendante → politique par capcode → SQLite et métadonnées → deux tableaux → deux TXT → voix avec chiffres séparés → WAV PCM/manifeste JSON. Politique NUMERIC conservée après redémarrage. AUTO et INCOMPLETE restent visibles/conservés sans voix ni job Pager. Double-clic testé avec lecture des candidats et mots radio. Les autres suites couvrent fichiers/profils, reprise SQLite/TXT, historique paginé, capcodes et migration, voix, WAV et transport LAB injecté.

La première exécution restreinte échouait à ouvrir les bases temporaires ; elle n'est pas un résultat applicatif réussi. L'exécution Windows native autorisée passe les dix suites, 64,17 secondes. Les bancs integration_tests et pager_tests passent aussi avec la plateforme Windows native ; captures contrôlées des deux tableaux et du détail numérique dans qualification-0112/captures. Essai RF de 300 secondes : 143 392 768 octets IQ, 1 297 trames, 109 messages SQLite et dans chaque tableau, 327 lignes TXT dans cinq destinations, contenu exact octet pour octet. Trois verrous SQLite injectés, 95 reprises, zéro poll IQ périmé et aucune écriture TXT restante. Trois annonces synthétiques SAPI et trois WAV pendant la réception ; aucune annonce de texte radio privé. Rapport : qualification-0112/live-rtl-300s.json. Résultats CTest finaux : qualification-0112/ctest.log.

## Limites et provenance de livraison

Le format applicatif des données belges compactes reste inconnu. AUTO préfère l'ambiguïté à une annonce trompeuse ; il faut sélectionner une politique explicite pour un émetteur connu. Une ALPHA forcée valide syntaxiquement peut contenir des données applicatives peu lisibles : la syntaxe ne prouve pas le sens. Les espaces NUMERIC finaux sont intrinsèquement ambigus. Les anciens messages SQLite n'ont pas de bits bruts rétroactifs.

Voix SAPI, WAV/JSON et relais préparé conservés. Aucun raccordement au serveur officiel PrivateRadio, aucune nouvelle API officielle inventée. Pas de qualification exhaustive sur tous les émetteurs, matériels ou Windows 10. Exécutables non signés.

Compilation locale Windows x64 ; l'état GitHub Actions sera rapporté séparément, sans assimiler un paquet local à un artefact Actions.
