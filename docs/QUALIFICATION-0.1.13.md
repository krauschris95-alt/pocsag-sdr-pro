# Qualification POCSAG SDR Pro 0.1.13

Base : v0.1.12, 771307d098666bf5c2a0dab46d922222270e7eda. Correction des politiques et du diagnostic, sans changement des bits ALPHA/NUMERIC, du BCH, de la synchronisation, du filtre FM, des deux tableaux, de leur ordre ou du filtrage des capcodes. Migration additive SQLite v10 : provenance des nouveaux enregistrements.

## Comparaison des versions, faits établis

Les moteurs des tags v0.1.10, v0.1.11 et v0.1.12 ont été extraits directement de Git et recompilés séparément. Trois captures IQ privées identiques par comparaison, 70 874 112 octets au total, démodulation 240 000 éch/s, mêmes paramètres. La capture de 60 secondes est certifiée 169,626 MHz ; aucune fréquence non prouvée n'est attribuée aux deux autres fichiers historiques. Scripts : compare_versions.py, qualify_numeric.py ; rapports agrégés qualification-0113/version-comparison.json et independent-comparison.json.

Les 62 charges utiles complètes sont exactement identiques dans leurs représentations ALPHA et NUMERIC entre 0.1.10, 0.1.11 et 0.1.12. Le nouveau code conserve également ces textes exacts ; seuls classement/diagnostic diffèrent. Quatre préfixes incomplets supplémentaires sont conservés par 0.1.12 et la nouvelle version, alors que les anciens moteurs les abandonnaient. Aucune corruption de l'extraction ALPHA introduite par NUMERIC n'est mise en évidence sur ces captures. Cela ne prouve pas l'absence de tout défaut sur un signal non testé.

Les six chaînes Base64 canoniques à octets non textuels sont déjà produites par 0.1.10 et 0.1.11, dont cinq sur la capture 169,626 MHz. Ces cinq chaînes concordent aussi avec multimon-ng. Elles ne sont donc pas créées par les nouveaux nibbles NUMERIC. Aucune signature gzip/ZIP/PNG prise en charge n'est identifiée dans ces cinq charges ; le format applicatif reste inconnu. Base64 conforme n'est pas une identification du protocole de l'émetteur. Aucun déchiffrement ni transformation en intervention n'est annoncé.

La régression de présentation est distincte de l'extraction : 0.1.12 remplace le texte générique des diagnostics AUTO par un candidat brut, rend AMBIGUOUS visible et conserve des préfixes incomplets auparavant abandonnés. Son AUTO ne sélectionne aucun texte ALPHA, même plausible, et supprime donc la voix de ces messages. Les anciens AUTO étaient déjà restrictifs et produisaient des diagnostics souvent masqués ; il ne s'agit pas d'un ancien AUTO universellement performant que l'on aurait restauré. Les anciennes préférences ALPHA restent conservées ; sur la configuration accessible, ALPHA est enregistré. La cause de chaque événement utilisateur particulier n'est pas établie.

## Politique corrigée

Priorité : règle explicite du capcode ALPHA/NUMERIC/TONE, puis politique réseau globale pour AUTO/INHERIT. En AUTO sans règle explicite, sélection ALPHA heuristique si syntaxe valide, au moins trois lettres, proportion de symboles limitée, présence de mots/espaces ou longueur compacte limitée à 24 caractères. L'incertitude et l'alternative NUMERIC restent dans le diagnostic. Les cas longs compacts, contrôles non supportés ou Base64 restent AMBIGUOUS. Aucune règle basée uniquement sur fonction 0–3. Un format NUMERIC connu est choisi une fois par capcode/réseau, pas par message.

Base64 : contrôle alphabet, longueur multiple de quatre, remplissage terminal et bits inutilisés canoniques, au moins 24 caractères / 16 octets. Analyse UTF-8 et contrôles sur les octets décodés ; signatures gzip/ZIP seulement signalées, sans extraction/décompression. En ALPHA/AUTO, une charge compatible Base64 reste originale, marquée AMBIGUOUS avec diagnostic et sans voix/WAV. Un capcode explicitement NUMERIC conserve la priorité de sa règle ; un choix manuel erroné ne peut pas être identifié avec certitude à partir du seul signal.

Sur la capture de 60 secondes : AUTO classe désormais 44 textes ALPHA, six charges ambiguës dont cinq Base64 et un préfixe incomplet ; 434 adresses TONE. Les anciens textes ne sont pas réécrits. Les messages binaires à contrôles non lisibles ne sont pas annoncés. Les caractères binaires qui imitent parfaitement un texte restent intrinsèquement impossibles à distinguer par syntaxe seule : l'heuristique n'est pas une garantie sémantique.

## Inspection et connexion utilisateur — limite non résolue

Lecture seule Python SQLite et pilote Qt identique à l'application : le fichier accessible au dossier confirmé mesure 32 768 octets, schéma user_version 0, table messages vide et dépourvue d'event_uid/métadonnées. Seul autre état SQLite trouvé dans le dossier et ses sous-dossiers : SHM 32 768 octets, WAL vide. Aucun autre fichier de base, ni redirection storage-location.json observé. Les parents du chemin n'exposent pas de lien/reparse point. Aucune commande de réparation, déplacement, checkpoint, réinitialisation ou écriture de données utilisateur n'a été exécutée.

Le journal technique indique pourtant 7 199 lignes supérieures / 16 inférieures et la même chaîne de chemin. Les outils de processus ne voient aucune application POCSAG correspondante ; le contrôleur de fenêtres n'a pas pu s'initialiser (chemin introuvable). Le même chemin déclaré ne prouve donc pas que l'image accessible aux outils correspond à la connexion de la fenêtre utilisateur. La cause système de cette divergence n'est pas établie. Aucun diagnostic de corruption, perte ou base vide de la fenêtre réelle n'est déduit de ce résultat.

Corroboration complémentaire après indication des horaires par l'utilisateur : deux lignes du TXT général correspondent exactement au capcode, aux dates/heures 05/11/2026 13:10:53 et 13:11:52 et au préfixe signalés. Le texte identique de 116 caractères est du Base64 canonique représentant 85 octets non textuels, sans signature applicative identifiée. Ces deux lignes sont antérieures à la publication de 0.1.12 le même jour à 18:54 locale. Leur ancienneté explique normalement l'absence de métadonnées ajoutées par cette version ; leur association précise à l'UUID reste à confirmer dans la connexion réelle. Aucun texte, capcode ou UUID de ce cas utilisateur n'est inclus dans les preuves publiées, uniquement les résultats agrégés de user-legacy-check.json.

L'événement signalé n'a pas pu être retrouvé dans la connexion réellement affichée. Son antériorité à la migration et l'origine de ses métadonnées absentes restent non déterminées. Cette vérification n'est pas déclarée terminée. Le nouveau dialogue Diagnostic de la connexion, dans Emplacement des données SQLite, interroge directement db_ depuis la fenêtre : PID/exécutable, connexion/pilote, fichiers DB/WAL/SHM, PRAGMA database_list, schéma, lignes et recherche UUID. Toutes ses requêtes sont SELECT ou PRAGMA de lecture ; le test compare total_changes() avant/après.

L'inspection affiche une explication pour chaque champ absent. Une provenance radio/source-sans-brut est enregistrée pour les nouveaux messages. Les anciennes lignes v0.1.12 ou antérieures sans provenance ne permettent pas de reconstituer les bits ni de dater la migration à partir des seuls champs vides. L'analyse Base64 du texte d'une ancienne ligne est possible sans modifier le message et sans prétendre remplacer les données IQ absentes.

## Tests et limites

Six fixtures IQ indépendantes : ALPHA/NUMERIC à 512/1200/2400, contenu numérique étendu exact. BCH, parité, fonctions, polarités, TONE, fragments, contrôles binaires, erreurs non corrigeables et interruptions conservés. Comparaison multimon-ng : 49 textes concordants dans chaque représentation, un message supplémentaire non corroboré comme en 0.1.12 ; cet écart reste signalé.

Intégration : Base64 synthétique réellement passé par l'assembleur POCSAG, conservation texte/bits/mots, politique ALPHA → ambiguïté sans voix ni job Pager ; NUMERIC indépendant → TXT/SQLite/voix séparée/WAV/JSON ; inspection legacy expliquée ; diagnostic connexion sans écriture SQL ; règle réseau et persistance après redémarrage. Les suites existantes couvrent R-MED1, R-MED2, CIP112, voix, profils, reprises et historique.

Dix suites CTest réussies en 57,51 secondes, avec fichiers et bases temporaires. Essai RTL réel 300 secondes : 143 425 536 octets IQ, 631 trames, 49 messages dans SQLite et chacun des deux tableaux, 147 lignes TXT dans cinq destinations, contenu exact octet pour octet. Trois verrous SQLite, 61 reprises, aucun poll IQ périmé ni écriture TXT restante. Trois annonces SAPI synthétiques et trois WAV produits pendant la réception ; aucune annonce de contenu radio privé. Voir live-rtl-300s.json. Les contrôles ZIP/installateur et la provenance Actions seront complétés après leur exécution. Aucun serveur officiel PrivateRadio modifié. Aucun texte radio privé, IQ, base utilisateur ou WAV d'intervention publié.

## Documentation du réseau

ASTRID décrit un service de paging crypté : [description officielle du service](https://www.astrid.be/sites/public/files/2025-10/paging_encryption_paging_fr_0.pdf), [annonce officielle](https://www.astrid.be/fr/actualites/chiffrement-pour-tous-les-pagers). Cela rend possible la coexistence de contenus clairs et protégés ; aucune attribution de ces 85 octets à ce service, à un algorithme ou à une clé n'est prouvée. Les aperçus indexés ont été consultés ; l'accès direct à ces deux ressources a renvoyé HTTP 403. Aucun décodeur applicatif n'est proposé sans spécification et preuve de correspondance.

## Livrables Windows x64

Code applicatif compilé : 70bea989678b4069d557adecbe874288844c2ea1. Le tag final ajoute uniquement les contrôles de livraison et la provenance ; aucun changement applicatif après compilation. Les documents intégrés aux binaires ont été figés avant ces contrôles.

ZIP : CRC intégral, extraction neuve, lancement code 0 avec PATH limité à Windows et données temporaires. Installateur : dossier neuf isolé, 211 fichiers tous identiques par SHA-256 au répertoire préparé, lancement code 0 sans Qt/MinGW de développement, désinstallation code 0 conservant le journal témoin et retirant l'application. Aucun remplacement d'une installation utilisateur. Rapports : portable-test.json, installer-test.json, release-assets.json.

SHA-256 ZIP : 2fdb24e494422c6280b268709016012e85d3a7449cf021eae6d4ec35d1baabac.
SHA-256 installateur : dff226041d26db56441727e5526801e249126943d2f67ccb05e43ed707e95fd9.

GitHub Actions : run 42102252204 du commit applicatif, échec avant démarrage du build : « The job was not started because an Actions budget is preventing further use. » Aucun artefact Actions, aucun réglage de facturation modifié. Les exécutables sont compilés et qualifiés localement.

L'analyse du contenu Base64 et les corrections de classement/inspection sont validées selon les tests décrits. La cause système de la divergence d'accès SQLite et le rattachement exact de l'UUID ne sont pas déclarés résolus. Le dialogue de diagnostic ouvre la voie à une vérification depuis la connexion réellement affichée ; il ne répare ni ne remplace les données.
