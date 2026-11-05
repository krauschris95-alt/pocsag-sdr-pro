# Qualification 0.1.15 — Clair Professionnel — Bleu Glacier

## Portée

Refonte de présentation du logiciel existant, sur le code 0.1.14 qui conserve les corrections apportées depuis 0.1.12. Aucun changement des algorithmes SDR, POCSAG, BCH, interprétation, ingestion ou formats TXT/WAV/JSON. Les données de validation sont temporaires et synthétiques ; aucune copie de l’historique privé n’est publiée.

## Comparaison avec la maquette

- Fond #F0F4F8, barre supérieure #F8FBFF, tableaux blancs alternés #E8F2FC, en-têtes #DCEAF7, titres #24649A et sélection #B7DCFA.
- Menus existants conservés dans l’ordre demandé. Bouton Lecture vert, Stop rouge, icônes bleues dessinées en code, bouton voix avec commandes fonctionnelles, accès capcodes et enregistrements.
- Deux tableaux superposés et séparateur mémorisé. Titres identiques à la demande ; lignes normales de 29 px, texte Segoe UI de 15 px, capcodes gras et dates/heures bleu gris. Source SDR facultative toujours masquée par défaut.
- Fréquence réelle/configurée dans un encadré à droite, mode et débit du récepteur sélectionné. Voyant basé sur l’arrivée IQ et qualité BCH réelle conservés ; aucune activité ni valeur à 100 % n’est simulée.
- Alias en badges vert/bleu selon la couleur configurée, avec texte assombri pour le contraste. Noms et couleurs ne sont pas codés en dur par alias.
- Barre inférieure : connexion SQLite interrogée, dossier en chemin abrégé avec infobulle complète, états TXT indépendants, génération WAV/JSON selon capcodes actifs, worker et destination disponible ; voix selon coupure générale réelle. Aucun compteur décoratif. Les diagnostics restent dans le journal et les erreurs conservent leur texte complet en infobulle.
- Choix clair/sombre et Compacte/Normale/Grande sauvegardés immédiatement et restaurés. Le clair est le défaut pour les anciennes configurations sans choix de thème.
- Fenêtres secondaires héritant de la palette ; interrupteurs des capcodes redessinés pour le fond clair. Spectre/waterfall conservés sur fond sombre afin de préserver la lisibilité des signaux, avec commandes claires.

## Différences explicites

Les boutons « Derniers messages » restent disponibles dans les titres. Aucun bouton caméra fictif n’est ajouté. Les états de service ne copient pas les ON de l’illustration. Le texte long est abrégé dans la cellule et reste accessible par infobulle ou double-clic. La barre de titre reste native Windows ; sa couleur est demandée par DWM lorsque la version de Windows la prend en charge. Les captures QWidget montrent la zone cliente, sans le cadre natif.

Les essais de dimension utilisent de vraies fenêtres Qt Windows redimensionnées ; les facteurs Qt 1, 1,25 et 1,5 simulent la mise à l’échelle de l’application. Ils ne constituent pas une modification ni une vérification des réglages DPI système de plusieurs moniteurs physiques. Les dimensions demandées 1366×768, 1600×900, 1920×1080 et 2560×1440 sont converties en unités logiques selon le facteur. Windows limite ensuite les grandes fenêtres à l’écran disponible : la zone cliente maximale observée est environ 1924×1061 pixels à 100 %. Les fichiers de capture portent la dimension demandée ; les journaux scale100/125/150/test.log donnent la dimension réellement obtenue. La résolution physique 2560×1440 n’est donc pas qualifiée sur ce poste. À 1366×768 et 150 %, les tableaux disposent de moins de lignes mais conservent leurs défilements et la fréquence reste visible.

## Résultats

- 11/11 suites CTest réussies en 69,44 s : décodeur, références indépendantes, ingestion, filtres, deux tableaux et défilement, reconnexion injectée, SQLite et reprises, TXT, voix et WAV/JSON, plus présentation.
- Test de présentation Windows réussi à facteurs Qt 1, 1,25 et 1,5 : dimensions réelles consignées dans les logs, fréquence dans la fenêtre, tailles 24/29/34 px, palettes et redémarrage avec 40 messages SQLite conservés.
- Captures réelles dans scale100/, scale125/ et scale150/ : tableaux, capcodes, journaux, voix, PrivateRadio, spectre et ancien thème sombre.
- Comparaison visuelle effectuée sur les captures Windows ; correction des icônes opaques et des interrupteurs des capcodes initialement sombres. L’ancienne assertion de géométrie excluait les nouveaux titres ; elle les inclut désormais en conservant le contrôle du rapport 60/40. Le minimum de hauteur des tableaux est vérifié à 100 px, y compris dans le petit écran à 150 %.
- RTL-SDR réel : 300 s à 169,626 MHz, 143 425 536 octets IQ, 1009 trames décodeur, 113 messages SQLite (dont trois annonces synthétiques), 113 lignes dans chaque tableau. Dernier identifiant SQLite égal à la dernière ligne des deux modèles, toutes deux visibles.
- 339 lignes TXT exactes dans cinq destinations simultanées ; zéro file restante. Trois verrous SQLite injectés, 95 reprises contrôlées, zéro contrôle IQ périmé.
- Trois annonces SAPI et trois WAV synthétiques terminés pendant la réception ; acquisition IQ continue. Test sur base, capcodes et journaux temporaires uniquement.
- Des avertissements d’énumération USB apparaissent dans les essais graphiques réalisés pendant que le banc matériel occupe la clé ; ils ne sont pas masqués. Le test de réception continue et ses mesures réussissent. Un avertissement PLL au démarrage n’entraîne pas de rupture IQ pendant l’essai.
- La dernière retouche après l’essai matériel concerne uniquement le dessin abrégé des libellés de la barre d’état ; elle préserve leur valeur complète pour l’accessibilité et les infobulles. Rendu et régressions revérifiés après cette retouche.

Contrôle des pixels de la capture native : les six couleurs principales de la palette sont présentes, chacune sur plus de 100 pixels (palette-check.json).


## Paquets livrés

ZIP extrait dans un dossier neuf et lancé avec PATH limité à Windows, exécutable identique par SHA-256 à la construction testée. Installateur : 223 fichiers installés comparés par SHA-256 au paquet, démarrage réussi, puis désinstallation conservant un journal TXT témoin. Aucun remplacement d’installation utilisateur.

La palette claire a été vérifiée dans les captures produites par l’exécutable extrait ET par l’exécutable installé (delivered-theme-check.json). Les exécutables contiennent effectivement le thème ; les captures ne sont pas une maquette générée. Les empreintes des deux téléchargements sont dans release-assets.json.

La publication automatique conserve désormais une préversion déjà publiée et qualifiée, pour éviter qu’un travail CI ultérieur remplace silencieusement les binaires dont les empreintes ont été vérifiées. Les nouveaux tags sans préversion existante conservent leur publication automatique et les artefacts Actions restent produits par la compilation.


## Première compilation distante

GitHub Actions a effectivement repris : run 42116638811, source 624ffe2. Configure, Build et Test réussis sur Windows-2022 ; Deploy échoue ensuite dans l’assemblage Python 3.15.2 avec module backports.zstd absent, puis compilateur NSIS introuvable. Aucun artefact publié par ce premier run. Le workflow fixe désormais Python 3.12 (même branche que le runtime local 3.12.14) et arrête immédiatement les commandes natives en échec. Les paquets de la préversion restent les binaires locaux déjà qualifiés ; la réussite du nouvel essai CI doit être vérifiée séparément.


Relance sur le tag : run 42118262661, compilation réussie puis 10/11 suites, échec du test de sélecteur dans sqlite_storage_regression (ligne 54), avant le packaging. Le même test passe localement et lors du premier run distant ; son automatisation changeait le backend natif pendant le dialogue modal. Le banc utilise désormais l’attribut Qt de dialogues non natifs avant le démarrage de QApplication, et fixe le dossier avant sélection. Les options du sélecteur de production continuent d’être vérifiées (AcceptSave, AnyFile, pas de confirmation d’écrasement et option native autorisée). Aucun changement de l’application ni des binaires publiés. Test ciblé relancé avec succès localement ; nouvelle CI sur main pour vérifier l’environnement Python et l’assemblage.


Run 42120533500 : compilation et onze suites de tests réussies sur le runner, puis échec explicite du packaging car aucune distribution binaire backports.zstd n’est disponible dans l’environnement pip du runner. Le test du sélecteur est donc stabilisé. Le packaging utilise désormais le chemin Python explicite fourni par setup-python, version 3.14, et le [module Zstandard standard introduit en Python 3.14](https://docs.python.org/3.14/library/compression.zstd.html), avec conservation du repli historique pour les anciens runtimes locaux. Aucun paquet externe Zstandard n’est requis dans la CI. Les exécutables de l’application restent inchangés.


## Résultat CI final

[Run 42121747722 réussi](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/42121747722), commit bf13e442b4d027aa572f75255ea915cdc1725985 : configuration, compilation Windows x64, tests, construction des dépendances SDR, NSIS et dépôt des artefacts réussis. Durée totale 7 min 19 s. [Artefact Windows téléchargeable](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/42121747722/artifacts/13458423178), 98,8 Mo affichés, SHA-256 7cadfe84ec0344b61cbfbe986a3f477f1719f47746580106ca4d314510b81129. Le travail release est normalement ignoré sur main ; la préversion v0.1.15 est déjà publiée.

Les fichiers de GitHub Releases conservent leurs empreintes locales qualifiées. Le commit de la CI ne change aucun fichier src/ par rapport au code applicatif 624ffe2 : uniquement banc de test, scripts de packaging, workflow et rapports. Le tag v0.1.15 reste immuable ; le code source avec les corrections de CI est disponible sur main et au commit bf13e44.

Le garde-fou de publication a été testé localement sans réseau : préversion existante → pas de remplacement, nouvelle préversion → publication autorisée, échec de lecture de l’API → arrêt sans autorisation de publication (release-guard-check.json).
