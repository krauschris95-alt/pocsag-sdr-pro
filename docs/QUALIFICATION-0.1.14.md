# Qualification 0.1.14

## Défaut reproduit

Test isolé avec 90 messages synthétiques : trier le tableau supérieur par Message croissant et le tableau inférieur par Heure décroissante, enregistrer les préférences, fermer et rouvrir, puis recevoir « AAA NOUVEAU SYNTHETIQUE ». Avant correction, le dernier élément du tableau ne correspond pas à cette nouvelle réception (échec du test restoredLiveOrder). Les en-têtes Qt mémorisent le tri en plus des dimensions des colonnes.

Le journal technique utilisateur montrait encore des décodages et des insertions SQLite, avec file d’ingestion vide, durant l’incident signalé. Ce constat établit que le voyant SDR n’était pas la seule preuve d’activité ; il ne démontre pas l’état exact de sa fenêtre, inaccessible aux outils de processus. Le défaut de tri est reproduit indépendamment sur une base temporaire. L’écart entre la base utilisateur visible aux outils et celle de sa fenêtre demeure une limite distincte de l’analyse précédente.

## Correction

Après restauration complète des en-têtes, imposer le tri par identifiant d’arrivée sur les deux modèles et retirer l’indicateur de tri de colonne. Rejoindre le bas après le premier calcul de disposition. Les commandes Derniers messages synchronisent le modèle et l’en-tête. Synchroniser aussi la coche Défilement automatique avec la préférence réellement chargée.

Le test de reprise utilise le rafraîchissement différé de production (150 ms), vérifie le texte de la dernière ligne des deux tableaux et la position de défilement. Les tests existants vérifient qu’une consultation manuelle de lignes anciennes n’est pas déplacée par une réception.

## Résultats

- 10/10 suites CTest réussies, 65,50 s, y compris restauration des tris et rafraîchissement différé de production.
- RTL-SDR réel à 169,626 MHz pendant 300 s : 143 425 536 octets IQ, 989 trames décodeur, 76 messages stockés dans la base temporaire et présents dans chacun des deux tableaux.
- Dernière ligne des deux modèles égale au dernier identifiant SQLite et visible dans la zone affichée : quatre contrôles réussis. Les 76 messages incluent trois annonces synthétiques ; aucune capture radio privée n’est publiée.
- 228 lignes TXT exactes dans cinq destinations simultanées ; zéro écriture restante.
- Trois verrous SQLite injectés, 90 reprises contrôlées ; aucun arrêt IQ détecté.
- Trois annonces SAPI et trois WAV de test synthétiques terminés, IQ continu pendant la lecture.

Le test matériel utilise une base, des TXT et des capcodes temporaires ; aucun fichier utilisateur n’est déplacé, effacé ou réinitialisé. La validation matérielle porte sur le chemin corrigé de restauration du tri ; les dernières modifications synchronisent uniquement la coche du menu et renforcent la même remise en ordre. Le moteur SDR/POCSAG est inchangé.

Un avertissement pilote « PLL not locked » apparaît à l’initialisation, mais la réception et tous les contrôles ci-dessus réussissent ; ne pas interpréter ce seul avertissement comme une perte de flux prolongée.


## Distribution

ZIP extrait dans un dossier neuf et lancé avec PATH limité à Windows : succès, données temporaires. Installateur testé dans un dossier isolé, comparaison SHA-256 de tous les fichiers installés, démarrage puis désinstallation avec conservation du TXT témoin : succès. Rapports JSON et empreintes des deux binaires dans qualification-0114/.

Compilation Windows x64 locale. GitHub Actions, run 42108992692 sur e9ebe9a, bloqué avant lancement du travail par le budget Actions : « The job was not started because an Actions budget is preventing further use. » Aucun artefact Actions ; aucune modification de facturation. La préversion fournit les binaires compilés et testés localement.
