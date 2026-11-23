# Qualification de la navigation dans l’historique

Essais locaux du 23 novembre 2026 sous Windows x64. Toutes les données de ces mesures sont synthétiques.

| Requête chronométrée | 100 000 messages | 1 000 000 messages |
| --- | ---: | ---: |
| Recompter le tableau supérieur | 30 ms | 393 ms |
| Recompter le tableau inférieur | 20 ms | 214 ms |
| Lire une page triée au milieu de la liste | 98 ms | 1 032 ms |
| Compter le dernier message ajouté | 0,07 ms | 0,09 ms |

Mesure SQLite en mémoire, avec 32 capcodes, une jointure sur les RIC convertis en entiers avec les index messages_ric et capcode_numeric_ric, un filtre de types et les bornes de dates du tableau supérieur. Ces chiffres caractérisent le coût des requêtes avant leur déplacement hors de la boucle Qt. Le test Debug history_navigation a observé un intervalle maximal de 5 ms puis 6 ms entre les battements de la boucle durant deux recherches successives. Il vérifie aussi que la demande de peinture revient avant le chargement de la page, que la recherche la plus récente gagne et que la base conserve son intégrité.

Le scénario global_corrections_tests --navigation-load 100000 30 exerce les deux QTableView, le tri, la recherche, l’ouverture des détails, les insertions et la journalisation TXT. Le premier essai a observé 69 nouvelles lignes et sept ouvertures de détails avec un intervalle maximal de 1 232 ms. Une répétition instrumentée a mesuré 1 334 ms dans l’appel de réception. Le critère de réactivité de 200 ms pour la réception simultanée exige une validation distincte des mesures sur le modèle seul.
Le profilage de la réception sur le disque E: a mesuré, dans un même appel de 1 472 ms, 632 ms de spool et 840 ms de stockage. Le worker utilise une connexion SQLite propre à son thread. Une épreuve a maintenu un verrou d’écriture jusqu’à la fermeture de la fenêtre, puis vérifié au redémarrage les 12 identifiants uniques dans leur ordre initial, les 12 lignes TXT et l’intégrité de la base. La réintroduction d’un spool déjà validé par SQLite a produit zéro doublon.

La mesure par événement Qt a également identifié des écritures de diagnostic sur le thread principal et un enregistrement périodique des préférences à 214 ms. Les diagnostics et la sauvegarde périodique des préférences utilisent chacun une file séquentielle. La fermeture attend les écritures de préférences avant la sauvegarde finale.