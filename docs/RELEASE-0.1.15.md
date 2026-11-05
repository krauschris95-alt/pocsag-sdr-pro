# POCSAG SDR Pro 0.1.15 — Clair Professionnel — Bleu Glacier

Refonte graphique selon la maquette claire : palette bleu glacier, titres et en-têtes bleus, deux tableaux superposés, lignes plus lisibles, alias colorés, commandes et fréquence regroupées, fenêtres secondaires claires. Le thème sombre reste disponible dans Affichage → Thème ; trois tailles de texte sont mémorisées.

Les états SDR/BCH, SQLite, TXT, WAV/JSON et voix utilisent les informations réelles. Aucun changement du décodeur, du filtrage des capcodes ni des formats de données. Historique et réglages utilisateur conservés.

Voir docs/QUALIFICATION-0.1.15.md pour les captures, les tests et les différences restantes.


Réception réelle RTL-SDR pendant cinq minutes : 113 messages SQLite, nouveautés visibles dans les deux tableaux, 339 lignes TXT exactes, reprise de trois verrous SQLite, trois annonces SAPI et trois WAV de test simultanés. Captures réelles et essais de palette/police à facteurs Qt 100/125/150 %. Pas de qualification physique en 2560×1440, écran indisponible.


Validation finale : 11/11 suites locales (69,44 s), ZIP portable et installateur vérifiés avec le thème clair effectivement présent dans les exécutables. [GitHub Actions réussi](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/42121747722) et [artefact CI téléchargeable](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/42121747722/artifacts/13458423178). Les téléchargements de la préversion restent les paquets locaux qualifiés. Le code applicatif est identique ; les derniers commits corrigent seulement le packaging CI et son banc de test.
