# POCSAG SDR Pro 0.1.7 — correction prioritaire SQLite

- Cause reproduite : curseurs persistants des tableaux + écritures TXT concurrentes → SQLITE_BUSY_SNAPSHOT 517. Les tableaux utilisent désormais des lectures paginées courtes, et les transactions de réception prennent leur verrou d’écriture avant leurs lectures.
- File de réception locale atomique et reprises après verrou temporaire ou redémarrage ; accusés TXT par position pour éviter les doublons après échec SQLite.
- Deux journaux indépendants : Belgique quotidien dans le dossier choisi et capcodes actifs/Journal TXT dans un fichier permanent. Format uniquement **CAPCODE DATE HEURE MESSAGE**. Anciens fichiers conservés sans conversion.
- Anciens messages en haut, nouveaux en bas ; suivi automatique uniquement depuis le bas. Historique chargé progressivement, sans plafond applicatif ni suppression automatique. Aucun compteur de messages dans l’interface.
- Deux tableaux PDW, menus, voix, SDR, filtres et spectre séparé conservés ; qualité mesurée sur les mots BCH/parité acceptés.

Validation : **8 suites réussies (23,78 s)** ; **23 minutes de RF réel à 169,626 MHz**, dont 20 minutes avec les deux journaux. 460 messages SQLite/chaque tableau, 443 lignes générales et 452 permanentes, contenu exact. 10 verrous SQLite injectés, 315 reprises, aucune file restante à la fin. Ces totaux reflètent les différentes phases ON/OFF. Voir `docs/QUALIFICATION-0.1.7.md`.

Préversion Windows x64, ZIP et installateur par utilisateur. Windows 10/AMD/autres matériels et H24 non qualifiés. Aucune transmission PrivateRadio ou cloud active.
