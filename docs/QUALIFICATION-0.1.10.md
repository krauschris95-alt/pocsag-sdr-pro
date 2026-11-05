# Qualification POCSAG SDR Pro 0.1.10

Base de développement : c1a1fa2c7a22dd077400443b1b00f909af2998be (0.1.9). Travail additif ; moteur SDR, démodulation et décodeur POCSAG conservés. Données de tests isolées ; aucun TXT, message radio privé ou base utilisateur dans les preuves publiées.

## Modifications

- Chemin permanent par capcode, choix existant/nouveau, validation d’écriture append et boutons Parcourir/Ouvrir. Ancien champ historique conservé séparément.
- Profils TXT nommés, ON/OFF, plusieurs capcodes cochés par profil et plusieurs profils par capcode, recherche numéro/alias, déduplication des chemins Windows normalisés.
- Journal quotidien Belgique indépendant. Conservation du fichier commun historique comme solution de compatibilité pour les capcodes sans chemin individuel.
- Voix autorisée au premier démarrage, coupure explicite persistante, sélection française automatique si aucune voix choisie. Alias puis message dans la file existante.
- Recherche numérique des anciens capcodes texte pour affichage, routage TXT et voix ; jointure numérique des anciens capcodes texte et séparation des filtres des deux tableaux. Effacement visible du filtre inférieur ; Derniers messages inférieur indépendant.
- Historique conservé sans purge, identifiants/offsets 64 bits, fenêtre visible progressive bornée (un million de lignes) et navigation vers tout l’historique.
- Migration explicite des données actives : sauvegarde SQLite VACUUM INTO, contrôle d’intégrité, copie avec SHA-256, activation atomique du localisateur en dernière étape. Source conservée ; cible existante non vide refusée ; échec sans activation. Aucun déplacement de TXT externe.
- Le worker TXT parcourt aussi les lots suivants quand les 1 000 premières écritures restent bloquées. Les destinations défaillantes ne bloquent plus les destinations saines ; l’ordre par destination et les accusés durables sont conservés. Clés d’accusés normalisées avec compatibilité des anciennes clés.

## Vérification audio réelle

La sonde Windows WASAPI lit uniquement les niveaux de la sortie de rendu pendant un message SAPI synthétique français : aucune prise microphone, aucun fichier audio enregistré. Signal RMS 0,04268, crête 0,37907, silence initial RMS 0 ; périphérique non muet, volume Windows 59 %. SAPI terminé, voix française sélectionnée. L’utilisateur a confirmé « Oui, alias puis message audibles ». Voir qualification-0110/windows-audio.json et audibility-confirmation.json. Ceci qualifie le PC testé, pas tous les périphériques Windows possibles.

## Cas R-MED1

Un ancien capcode TEXT `02006314` et un message TEXT `2006314` reproduisent une jointure absente ; la jointure numérique rétablit la ligne et l’alias R-MED1. Les filtres généraux supérieurs ne masquent plus le tableau inférieur. Le test vérifie aussi un ancien filtre inférieur et son effacement. La base locale consultée en lecture seule ne contenait pas le capcode ni les messages signalés : il ne serait pas exact d’attribuer avec certitude la capture utilisateur à une seule de ces causes.

## Tests et limites

Tests automatisés : routage multi-fichiers/profils, déduplication, répétitions réelles, ON/OFF indépendant, fichier ancien conservé, troisième destination, verrou SQLite et reprise, arriéré de 1 100 écritures bloquées puis reprise intégrale, voix simulée exactement une fois, R-MED1, historique avec identifiants supérieurs à 2^40, navigation bornée, dialogues de profils/capcodes, migration vérifiée et échecs sans perte. Les suites existantes conservent les tests minuit belge, 100 001 messages, décodeur indépendant et reprise matérielle injectée.

Le sélecteur natif Windows est demandé par le produit ; les tests pilotent son remplacement Qt pour choisir les fichiers automatiquement. Windows 11 / Qt 6.8.3 / MinGW 13.1 x64. Windows 10, disque USB physique et partage réseau autorisé non qualifiés. Aucun changement de droits ou facturation. La jauge reste le taux BCH/parité mesuré, pas une qualité RF composite. PrivateRadio reste une préférence locale sans envoi réseau.

La migration nécessite de l’espace pour une sauvegarde et une copie complètes sur la destination. En cas d’échec, les copies partielles sont conservées pour inspection ; aucun nettoyage automatique des données. Après migration réussie, revenir à une ancienne copie ne fusionne pas les messages reçus depuis ; le dossier récent reste disponible.

Neuf suites automatisées réussies en 45,71 secondes (qualification-0110/ctest.log). Dialogues Windows : réussite, notamment sélecteur Qt de test, profils et migration (native-global.log). Captures réelles sous qualification-0110/captures, uniquement données synthétiques.

Premier essai RF : 1 200 secondes à 169,626 MHz, 575 422 464 octets IQ, 5 166 trames décodées avant filtrage, 448 textes acceptés et présents dans chaque tableau, 1 344 lignes dans cinq fichiers (Belgique, trois fichiers individuels, profil commun). Toutes les lignes correspondent octet pour octet à l’outbox ; aucune écriture en attente. Dix verrous SQLite de six secondes, 317 reprises. Une lecture SAPI terminée, les IQ continuant pendant la voix. Le contrôle de fraîcheur IQ a été négatif sur un poll de 100 ms : l’assertion stricte zéro-poll a donc échoué. La cause de ce bref état n’est pas établie et aucune absence de perte radio ne peut être prouvée par ces seuls compteurs. Les tests d’autres fenêtres énumérant les périphériques étaient exécutés en parallèle ; ceci n’établit pas leur responsabilité. Résultat brut conservé dans live-multifile.json.

Les régressions déterministes vérifient séparément le passage à minuit et les combinaisons de journaux OFF/général/capcodes/deux. La comparaison des TXT avec SQLite démontre l’absence de perte/duplication après acceptation, pas la réception de toutes les émissions de l’émetteur.

## Provenance de compilation

Code applicatif final : 8c4367bf5a40ad86771bedd8db573421eb2b6990. Le dernier correctif ajoute la recherche numérique des anciens RIC ; le test ciblé est réussi. L’essai de vingt minutes utilise le code 0bd573a ; un contrôle matériel complémentaire est exécuté sur le code final. Le workflow [42031451417](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/42031451417), job 127258658367, est refusé avant démarrage par le budget GitHub Actions. Aucun artefact Actions produit ; distributions compilées localement. Aucun changement de facturation effectué.

## Contrôle matériel final et moniteur de test

Le moniteur des deux premiers essais restait actif pendant `processEvents()` après l’arrêt volontaire du SDR. Ce poll postérieur à l’arrêt ajoutait un incident artificiel. La minuterie est désormais arrêtée avant la fermeture du récepteur ; aucun changement de moteur SDR. Un nouveau contrôle de 300 secondes sur le code applicatif final réussit : 143,392,768 octets IQ, 1087 trames avant filtrage, 92 textes acceptés dans chacun des deux tableaux, 276 lignes exactes dans 5 fichiers, 3 verrous injectés / 88 reprises, zéro poll IQ manqué, zéro TXT en attente, une lecture SAPI terminée avec IQ continuant pendant la voix. Résultat : live-final-corrected-monitor.json. Les rapports initiaux sont conservés pour expliquer la correction du banc, plutôt que remplacés par un résultat supposé.

Le test natif Windows des dialogues réussit après configuration du banc pour utiliser directement le sélecteur Qt : commuter le dialogue natif vers son remplacement en plein `exec()` était une méthode de test invalide et faisait échouer le banc. Le produit continue à demander le sélecteur Windows natif ; le test n’en pilote pas le shell Windows.

## Distribution finale

ZIP CRC vérifié, lancement depuis extraction neuve avec PATH Windows uniquement : code 0, données isolées. Installateur exécuté dans un dossier neuf du workspace après vérification de l’absence d’installation enregistrée ; installation, lancement et désinstallation : codes 0. Un TXT témoin non livré reste intact après désinstallation ; application retirée. Aucun essai sur l’installation ou les données utilisateur. Résultats portable-test.json, installer-test.json et empreintes release-assets.json. Les documents de la distribution sont figés avant ces essais ; le présent rapport du tag ajoute les résultats après assemblage.
