# POCSAG SDR Pro 0.1.14 — retrouver les nouvelles réceptions

Un tri de colonne enregistré dans l’en-tête (Message, Heure, etc.) pouvait être restauré au démarrage : les nouveaux messages étaient reçus et enregistrés, mais classés ailleurs que dans le bas du tableau. L’application rétablit maintenant l’ordre d’arrivée dans les deux tableaux au démarrage, tout en conservant les largeurs et la disposition des colonnes. « Derniers messages » rétablit aussi l’indicateur de tri. La position initiale rejoint le bas après la mise en page.

Le menu Défilement automatique reflète désormais son réglage effectivement mémorisé. La consultation manuelle de l’historique reste préservée. Aucun changement du décodeur, des filtres, des fichiers TXT ni de la base utilisateur.

Validation détaillée : docs/QUALIFICATION-0.1.14.md.


Validation : 10/10 suites CTest, reprise du tri testée avec rafraîchissement différé et affichage Windows. RTL réel pendant 300 s : 76 messages SQLite et deux dernières lignes visibles, 228 lignes TXT exactes dans cinq destinations, trois verrouillages repris, voix et WAV synthétiques simultanés. Base et TXT utilisateur préservés.

ZIP portable et installateur testés localement. GitHub Actions bloqué avant compilation par le budget du compte (run 42108992692) ; les binaires fournis sont issus de la compilation Windows x64 locale.
