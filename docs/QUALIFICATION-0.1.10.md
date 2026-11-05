# Qualification POCSAG SDR Pro 0.1.10

Base de développement : c1a1fa2c7a22dd077400443b1b00f909af2998be (0.1.9). Travail additif ; moteur SDR, démodulation et décodeur POCSAG conservés. Données de tests isolées ; aucun TXT, message radio privé ou base utilisateur dans les preuves publiées.

## Modifications

- Chemin permanent par capcode, choix existant/nouveau, validation d’écriture append et boutons Parcourir/Ouvrir. Ancien champ historique conservé séparément.
- Profils TXT nommés, ON/OFF, plusieurs capcodes cochés par profil et plusieurs profils par capcode, recherche numéro/alias, déduplication des chemins Windows normalisés.
- Journal quotidien Belgique indépendant. Conservation du fichier commun historique comme solution de compatibilité pour les capcodes sans chemin individuel.
- Voix autorisée au premier démarrage, coupure explicite persistante, sélection française automatique si aucune voix choisie. Alias puis message dans la file existante.
- Jointure numérique des anciens capcodes texte et séparation des filtres des deux tableaux. Effacement visible du filtre inférieur ; Derniers messages inférieur indépendant.
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

Les résultats finaux de réception prolongée et de distribution seront ajoutés après leur exécution.
