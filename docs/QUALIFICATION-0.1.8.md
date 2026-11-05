# Qualification POCSAG SDR Pro 0.1.8

Base : version 0.1.7, commit 8c03e09bf57af53fb0995e523f4168c72eca6c24. Aucun changement de l’algorithme de démodulation ou du décodeur POCSAG. Données utilisateur préservées ; essais avec bases et journaux temporaires isolés.

## Corrections

- Interrupteurs ON/OFF contrastés, cliquables au niveau de la cellule et utilisables au clavier ; capcodes compacts et paramètres avancés séparés.
- Commande vocale générale visible, réglages SAPI existants conservés, alias avant le texte complet, file indépendante et arrêt sans arrêter le SDR.
- Deux journaux indépendants conservés ; choix direct du TXT permanent, ouverture de son fichier/dossier et nom quotidien général explicite. Un fichier existant est ajouté sans dialogue trompeur de remplacement.
- Nouvelles lignes UTF-8 : `CAPCODE JJ/MM/AAAA HH:mm:ss MESSAGE`, sept chiffres pour le capcode. Les anciennes lignes et fichiers ne sont pas convertis.
- Thème noir/bleu, alternance discrète, en-têtes bleus, dates/heures et alias violets, mode/vitesse jaunes, textes cyan. Deux tableaux, spectre séparé, séparateur et largeurs mémorisés. Bouton Derniers messages et commande indépendante par tableau.
- Jauge BCH réelle, aucun pourcentage après trois secondes sans nouveaux mots ; avertissement espace disque faible sans purge.
- Reprise SQLite, spool atomique et reçus TXT de 0.1.7 conservés.

## Tests automatisés

Les huit suites CTest passent en 26,27 s : décodage, intégration, références indépendantes, file vocale, reconnexion injectée, sauvegarde des capcodes, stockage SQLite, reproduction du verrouillage historique 517.

Le test des interrupteurs envoie un événement de clic au délégué Voix, sauvegarde puis vérifie SQLite et la restauration. Les tests de réception avec backend vocal contrôlé vérifient l’alias avant le texte, l’absence de superposition et les ON/OFF indépendants. Les tests de stockage vérifient douze messages pendant un verrou, reprise et redémarrage sans doublon, erreurs de fichiers indépendantes, quatre combinaisons ON/OFF, minuit Bruxelles, format TXT exact, anciens fichiers conservés et historique de 100 001 lignes avec cache limité à 4 096 lignes. Cette limite concerne le cache uniquement.

Les captures Windows natives utilisent des messages synthétiques. Elles montrent les deux tableaux, les interrupteurs et la configuration des deux fichiers ; aucun message radio réel n’est publié.

## Voix Windows réelle

Microsoft Hortense Desktop (français) sélectionnée, lecture SAPI terminée sans erreur : `qualification-018/speech-windows.json`.

La chaîne complète sauvegarde un capcode avec Voix ON, active l’interrupteur général, reçoit un message synthétique via Window::received et attend le signal SAPI completed. ON/OFF général et insertion SQLite vérifiés : `qualification-018/automatic-speech.log`. Le journal d’essai et la base sont isolés.

## Essais matériels

Essai direct sur le RTL OEM R828D à 169,626 MHz, 240 kéch/s, gain demandé 42 dB : TXT OFF 60 s (29 textes), général seul 60 s (33), sélectionnés seul 60 s (36), les deux 1 200 s (502). Total : **600 messages SQLite et 600 lignes dans chacun des deux modèles**. Le banc inscrit automatiquement les capcodes reçus uniquement dans sa base temporaire pour exercer le panneau inférieur et le fichier sélectionné ; l’application ne fait pas cette inscription automatique. Les tests logiciels vérifient séparément l’exclusion des capcodes absents/inactifs et l’indépendance En bas/Journal TXT.

Fichier général : **535 lignes**, permanent : **538 lignes**, exactement les nombres attendus. Comparaison complète octet par octet avec l’outbox : identique. Dix verrous SQLite de six secondes, déclenchés sur des arrivées réelles : **283 reprises**, aucune attente IQ périmée observée, zéro message en spool et zéro TXT en attente à la fin. 661 962 752 octets IQ traités. Le voyant redevient inactif après arrêt. Rapport : `qualification-018/hardware-soak.json`.

Le pilote affiche au démarrage « PLL not locked! », puis les IQ et textes circulent normalement pendant l’essai ; aucune portée de qualification supplémentaire n’est déduite de cet avertissement. Les 20 032 mots mesurés acceptés ne constituent pas une preuve de capture de toutes les transmissions radio. Aucun message ou capcode réel n’est publié dans les preuves.

## Limites explicites

Windows 11 Pro 26H2, Intel i7-8550U, Qt 6.8.3 / MinGW 13.1 x64 disponibles sur ce PC. Windows 10 et autres configurations non testés. Pas de certification H24 ou de garantie contre panne physique/stockage plein. Les exécutables ne sont pas signés.

Le pourcentage est un taux BCH/parité de mots reçus, pas un indice composite calibré synchronisation/RF. RSSI/SNR et synchronisation stable de la maquette ne sont pas affichés faute de qualification de ces mesures. Le modèle Qt expose des indices int (limite structurelle de 2 147 483 647 lignes simultanément adressables par un modèle) ; SQLite conserve les identifiants 64 bits, sans purge ni plafond configuré. La qualification d’une navigation au-delà de cette plage reste à réaliser. Les alertes espace faible sont préventives et ne garantissent pas la persistance si le disque devient plein entre deux contrôles.

PrivateRadio : préférences indépendantes sauvegardées, aucun transfert réseau. La qualification matérielle des familles SDR autres que RTL-SDR reste inchangée.

## Compilation et Actions

Compilation locale Windows x64, Qt 6.8.3 / MinGW 13.1, mêmes bibliothèques desktop_window et desktop_services pour l’application et les tests. Le workflow mis à jour a été déclenché : [run 41997547900](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/41997547900). Il n’a pas démarré : « The job was not started because an Actions budget is preventing further use. » Aucun artefact de cette version n’a été compilé par GitHub Actions. Aucun changement de facturation effectué. Le workflow reste disponible pour une exécution lorsque le budget sera rétabli.

## Référence indépendante réexécutée

Comparaison avec multimon-ng, discriminateur et rééchantillonnage externes : un texte exact à chacun des débits 512, 1200 et 2400 sur ses enregistrements de référence. La capture RF privée déjà acquise pour 0.1.6 a été relue : 3 textes sur 3 strictement identiques, aucun manquant ou supplémentaire dans cet échantillon. Cette relecture est distincte de l’essai RF direct 0.1.8 et ne constitue pas une nouvelle capture. Résultats agrégés uniquement dans `qualification-018/independent-reference.json`.

## Installation préliminaire

Installateur NSIS compilé, installation silencieuse dans un dossier neuf du workspace, démarrage de l’application avec PATH limité aux dossiers système Windows, désinstallation : codes 0. Fichier utilisateur synthétique préservé, exécutable retiré. Aucun enregistrement d’installation utilisateur préexistant n’a été écrasé. Le binaire et le script d’installation restent identiques dans l’assemblage final ; seuls les documents de qualification sont complétés.

La persistance des largeurs des deux tableaux après réouverture et la modification/sauvegarde via le dialogue avancé sont également testées.

## Save et voix pendant la réception directe

Essai RTL supplémentaire de 60 secondes : Save accepté pendant que le thread SDR continuait ; lecture SAPI d’un texte synthétique terminée, 17 textes radio reçus et TXT vérifié, voyant inactif après arrêt. `qualification-018/save-speech-live.log`. La clé est libérée par les bancs après les tests. Aucun programme utilisateur n’a été arrêté de force.

L’application et le décodeur n’ont pas été modifiés pendant les essais prolongés ; seules les preuves, la documentation et des assertions de tests supplémentaires ont été complétées. Les empreintes de l’exécutable, du binaire de test, de desktop_window et du pilote sont dans `qualification-018/build-payload.json`.

## Validation des fichiers définitifs

Après assemblage : ZIP extrait dans un dossier neuf, contrôle CRC et SHA256 de tous les fichiers extraits, exécutable identique au binaire qualifié, démarrage avec PATH système seul : code 0. Installateur final exact installé dans un autre dossier neuf : installation, lancement et désinstallation codes 0 ; fichier utilisateur synthétique conservé et exécutable supprimé. Preuves `portable-test.json`, `installer-final-test.json` et empreintes `release-assets.json` dans `qualification-018/`.

Ces preuves finales ont été ajoutées au dépôt après l’assemblage ; le rapport inclus dans les paquets s’arrête à l’assemblage précédent. Aucun changement applicatif depuis le commit de code 538f50efba4d788b605d406c8ddee2dd92beecf6. Les archives ne contiennent ni base, ni préférences utilisateur, ni capture IQ privée, ni vidéo privée.
