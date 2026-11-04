# Validation et travail restant

## Tests automatisés implémentés

- Mot idle connu `0x7A89C197` pour vérifier l'encodeur BCH.
- Tous les masques de corruption d'un bit et de deux bits d'un mot : 528 cas.
- Trames à 512, 1200, 2400 bauds en polarité normale et inversée.
- Extraction RIC et fonction, texte ALPHA et appel TONE.
- Génération IQ CU8 FSK à ±4500 Hz : préambule, sync, adresse et message numérique
  connus ; démodulation en mode AUTO aux trois débits et deux polarités.

Ces tests sont synthétiques. Ils ne prouvent pas une réception physique ni une
résistance suffisante aux signaux faibles ou perturbés.

## Non validé / non terminé

- Aucun essai matériel RTL-SDR, V3, V4 ou HackRF n'a été effectué.
- Aucun fichier IQ provenant d'une capture radio indépendante n'est fourni.
- Aucun comparatif multimon-ng réalisé.
- HackRF, plusieurs récepteurs et plusieurs canaux simultanés non implémentés.
- Reconnexion automatique USB, démarrage Windows et politique de rétention non implémentés.
- Sauvegarde/restauration, purge et contrôles d'intégrité via interface à ajouter.
- Filtres fréquence/débit, tri interactif et couleurs d'alias dans les messages à compléter.
- Journal TXT : pas de file persistante de reprise ni transaction distribuée SQLite/TXT ;
  un arrêt entre les deux écritures peut laisser une ligne absente. Pas de journal quotidien.
- Gestion des capcodes et profils enregistrée par bouton Enregistrer, pas à chaque frappe.
- Type de message déduit de la fonction (0 numérique, autres alpha), convention non universelle.
- Synchronisation initiale exacte ; tolérance de deux erreurs sur sync aux frontières de batch.
- Filtrage DSP et récupération d'horloge initiaux, à qualifier sur captures et bruit.
- FFT réelle CU8, échelle relative dBFS ; pas de SNR ni de qualité fictifs.
- Spectre : zoom, sélection de fréquence, déplacement et bande passante visuelle à ajouter.
- Les tableaux sont adossés à SQLite mais leur rafraîchissement complet par message
  doit être optimisé avant qualification 24 h/24. Le DSP s'exécute dans le fil de réception.
- La fermeture peut attendre la fin d'une lecture USB synchrone.
- Les tests d'intégration SQLite, panneaux, filtrage, UTF-8, ajout sans effacement,
  routage multiple, déduplication des chemins, changement de fichier et restauration passent.
  Une qualification visuelle et prolongée de l'interface reste nécessaire.

## Exécution locale du 4 novembre 2026

Compilation Release x64 avec GCC 13.1 / Qt 6.8.3 : réussie.
CTest : 2 suites sur 2 réussies (moteur et intégration), environ 0,46 seconde.
Les tests Qt nécessitent l'accès normal aux fichiers temporaires ; le bac à sable local
empêche l'ouverture SQLite de certains chemins temporaires.

La version 0.1.0 est une base intégrée à tester. Elle ne doit pas être présentée comme
un successeur PDW finalisé ou une réception matérielle validée.
