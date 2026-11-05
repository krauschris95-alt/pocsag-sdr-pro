# Qualification POCSAG SDR Pro 0.1.9

Correction ciblée de la sélection des destinations TXT depuis 0.1.8 (84b068048a97ef878950b26bb43fe18c1c73c9ee). Décodeur, démodulation, requêtes et filtrage des deux tableaux, ingestion, worker TXT et format des lignes inchangés. Aucun effacement de données utilisateur.

## Corrections

- Fenêtre unique avec deux sections indépendantes, chemins éditables, boutons de sélection plus grands, commandes Ouvrir le fichier/dossier et ON/OFF explicites.
- Sélecteur demandé natif Windows, AcceptSave/AnyFile, suffixe .txt, ajout dans un fichier existant sans demande trompeuse de remplacement. Lettres de disque et chemins UNC autorisés, sans liste de lecteurs imposée.
- Accès direct depuis Capcodes, section permanente mise en évidence et champ focalisé ; les modifications non encore enregistrées du capcode restent dans leur dialogue.
- Vérification réelle d’écriture par fichier temporaire automatiquement supprimé et ouverture append/flush de la destination, sans troncature ni modification des anciennes lignes. Le fichier du jour peut être créé vide lors de la validation.
- Chemins absolus exigés : Qt acceptait C:relative.txt bien qu’il soit relatif au répertoire courant du lecteur C:. Cette forme est explicitement refusée, comme les noms Windows réservés et flux ADS involontaires.
- Préférences synchronisées avant application des destinations ; en cas d’échec, anciennes valeurs rétablies dans le cache et destinations actives conservées. Erreur précise, saisie conservée, aucun repli vers un dossier par défaut.
- Les messages déjà en attente avant un changement gardent leur ancienne destination ; le nouveau chemin s’applique aux nouvelles arrivées.

## Validation automatisée

**8/8 suites réussies en 23,08 secondes**. Résultats complets dans qualification-019/ctest.log et ctest-verbose.log. Les tests incluent les deux journaux simultanés, le passage à minuit belge, l’historique de 100 001 lignes et la reprise des verrous SQLite, sans changement des moteurs existants.

Tests de destinations : fichier existant conservé puis complété, création d’un nouveau fichier, UTF-8/accents/espaces, aucune rotation quotidienne du fichier permanent, préférences présentes sur disque immédiatement puis restauration après redémarrage. Chemins relatifs, C:relatif, NUL, caractère interdit, flux ADS, fichier verrouillé, dossier impossible et partage UNC local inexistant refusés, chemins actifs non remplacés. Les boutons Ouvrir refusent aussi les chemins vides/relatifs au lieu d’ouvrir le répertoire courant. Échec de sauvegarde INI injecté : erreur et maintien des valeurs précédentes.

Une lettre de lecteur supplémentaire est mappée temporairement par SUBST vers un dossier d’essai du workspace ; les écritures des deux journaux et la reprise après redémarrage sont vérifiées via cette lettre. Le mapping est retiré dans finally. Ceci qualifie l’adressage d’un autre lecteur, pas un disque physique D:/E: ou une clé USB spécifique.

Le bouton Choisir mon fichier TXT est exercé pour un fichier existant et un nouveau : le test vérifie le mode AnyFile/AcceptSave, la demande native et l’absence de confirmation d’effacement, puis pilote le dialogue Qt de remplacement pour sélectionner le fichier automatiquement. Le navigateur natif Windows n’est pas piloté par ce test automatisé.

L’accès depuis Capcodes est exercé : champ permanent focalisé, validation indépendante et saisie du capcode préservée même en annulant ensuite le dialogue Capcodes.

## Périmètre et limites

Windows 11, Qt 6.8.3 / MinGW 13.1 x64. Aucun partage réseau autorisé ni support externe disponible pour une qualification matérielle dédiée ; seule l’erreur d’un UNC inaccessible est testée. Windows 10 non essayé. Aucune modification de droits, création de partage réseau ou inscription d’identifiants effectuée. Pas de nouvel essai RF dans cette correction de chemins ; les essais RTL prolongés 0.1.8 restent documentés séparément. Le logiciel utilise les accès accordés au compte Windows et signale les erreurs ; il ne garantit pas l’accès à un disque absent ou à un partage interdit.

Captures natives avec données synthétiques uniquement. Aucun message radio, base utilisateur ou TXT utilisateur distribué.

## Compilation et provenance

Application Windows x64 compilée localement avec Qt 6.8.3 / MinGW 13.1 ; huit suites de tests exécutées avant assemblage. Le workflow [42019946766](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/42019946766) a été déclenché, mais GitHub refuse de démarrer le job : « The job was not started because an Actions budget is preventing further use. » Aucun artefact Actions n’a été produit et aucun changement de facturation n’a été effectué. Les distributions proviennent de la compilation locale, pas d’Actions.

Code applicatif : e304fab79793f800acd7b6746a80c0d8d015ba21. Les résultats des essais finaux des archives sont ajoutés au dépôt après leur assemblage ; aucune modification applicative n’intervient pendant cette phase.

## Distribution finale vérifiée

ZIP final : CRC et empreinte de chaque fichier extrait vérifiés, exécutable identique au binaire testé. Extraction dans un dossier neuf puis lancement avec PATH limité aux répertoires système Windows : code 0. Installateur final : installation dans un dossier neuf, démarrage et désinstallation codes 0. Le fichier utilisateur synthétique est conservé et l’exécutable retiré. Aucun enregistrement d’installation existant n’a été remplacé.

Preuves : qualification-019/portable-test.json, installer-test.json, build-payload.json, release-assets.json et captures. Ces derniers résultats sont ajoutés au dépôt après assemblage ; les rapports embarqués dans les archives s’arrêtent avant ces contrôles finaux. Aucun changement de code depuis e304fab79793f800acd7b6746a80c0d8d015ba21.
