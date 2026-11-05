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

Les huit suites CTest passent en 21,54 s : décodage, intégration, références indépendantes, file vocale, reconnexion injectée, sauvegarde des capcodes, stockage SQLite, reproduction du verrouillage historique 517.

Le test des interrupteurs envoie un événement de clic au délégué Voix, sauvegarde puis vérifie SQLite et la restauration. Les tests de réception avec backend vocal contrôlé vérifient l’alias avant le texte, l’absence de superposition et les ON/OFF indépendants. Les tests de stockage vérifient douze messages pendant un verrou, reprise et redémarrage sans doublon, erreurs de fichiers indépendantes, quatre combinaisons ON/OFF, minuit Bruxelles, format TXT exact, anciens fichiers conservés et historique de 100 001 lignes avec cache limité à 4 096 lignes. Cette limite concerne le cache uniquement.

Les captures Windows natives utilisent des messages synthétiques. Elles montrent les deux tableaux, les interrupteurs et la configuration des deux fichiers ; aucun message radio réel n’est publié.

## Voix Windows réelle

Microsoft Hortense Desktop (français) sélectionnée, lecture SAPI terminée sans erreur : `qualification-018/speech-windows.json`.

La chaîne complète sauvegarde un capcode avec Voix ON, active l’interrupteur général, reçoit un message synthétique via Window::received et attend le signal SAPI completed. ON/OFF général et insertion SQLite vérifiés : `qualification-018/automatic-speech.log`. Le journal d’essai et la base sont isolés.

## Essais matériels

Essai prolongé RTL-SDR à 169,626 MHz en cours de qualification. Les résultats agrégés seront consignés ici avant publication.

## Limites explicites

Windows 11 Pro 26H2, Intel i7-8550U, Qt 6.8.3 / MinGW 13.1 x64 disponibles sur ce PC. Windows 10 et autres configurations non testés. Pas de certification H24 ou de garantie contre panne physique/stockage plein. Les exécutables ne sont pas signés.

Le pourcentage est un taux BCH/parité de mots reçus, pas un indice composite calibré synchronisation/RF. RSSI/SNR et synchronisation stable de la maquette ne sont pas affichés faute de qualification de ces mesures. Le modèle Qt expose des indices int (limite structurelle de 2 147 483 647 lignes simultanément adressables par un modèle) ; SQLite conserve les identifiants 64 bits, sans purge ni plafond configuré. La qualification d’une navigation au-delà de cette plage reste à réaliser. Les alertes espace faible sont préventives et ne garantissent pas la persistance si le disque devient plein entre deux contrôles.

PrivateRadio : préférences indépendantes sauvegardées, aucun transfert réseau. La qualification matérielle des familles SDR autres que RTL-SDR reste inchangée.
