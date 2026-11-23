# Qualification de la navigation dans l’historique

Essais locaux du 23 novembre 2026 sous Windows x64. Toutes les données de ces mesures sont synthétiques.

| Requête chronométrée | 100 000 messages | 1 000 000 messages |
| --- | ---: | ---: |
| Recompter le tableau supérieur | 124 ms | 1 287 ms |
| Recompter le tableau inférieur | 195 ms | 2 130 ms |
| Lire une page triée au milieu de la liste | 212 ms | 2 072 ms |
| Compter le dernier message ajouté | 0,07 ms | 0,08 ms |

Mesure SQLite en mémoire, avec 32 capcodes, une jointure sur les RIC convertis en entiers et des types de messages filtrés. Ces chiffres caractérisent le coût des requêtes avant leur déplacement hors de la boucle Qt. Le test Debug history_navigation a observé un intervalle maximal de 5 ms puis 6 ms entre les battements de la boucle durant deux recherches successives. Il vérifie aussi que la demande de peinture revient avant le chargement de la page, que la recherche la plus récente gagne et que la base conserve son intégrité.

Le scénario global_corrections_tests --navigation-load 100000 30 exerce les deux QTableView, le tri, la recherche, l’ouverture des détails, les insertions et la journalisation TXT. Le premier essai a observé 69 nouvelles lignes et sept ouvertures de détails avec un intervalle maximal de 1 232 ms. Une répétition instrumentée a mesuré 1 334 ms dans l’appel de réception. Le critère de réactivité de 200 ms pour la réception simultanée exige une validation distincte des mesures sur le modèle seul.