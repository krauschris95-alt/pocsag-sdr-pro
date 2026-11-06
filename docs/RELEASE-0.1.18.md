# POCSAG SDR Pro 0.1.18 — spectre fixe −20 / −100 dBFS

Correction du spectre : mode **Échelle fixe** par défaut, maximum −20 dBFS et minimum −100 dBFS, neuf graduations dont la borne basse visible. Le spectre ne change plus de plage dans ce mode. Mode automatique facultatif et mémorisé séparément.

Waterfall bleu nuit/azur/cyan avec contraste et luminosité réglables, courbe cyan plus lisible et lissage temporel réglable (1 = désactivé). Configuration dans Affichage → Réglages spectre / waterfall, sauvegarde immédiate et restauration au redémarrage. La FFT conserve sa normalisation et ses valeurs brutes ; le lissage moyen de puissance agit uniquement sur la courbe, et les couleurs ne déplacent pas le bruit mesuré.

Décodeur POCSAG, capcodes, journaux TXT, SQLite, voix et PrivateRadio conservés. Voir docs/QUALIFICATION-0.1.18.md pour les résultats et les captures avant/après redémarrage sur RTL-SDR réel. ZIP portable et installateur Windows x64 non signés, faute de certificat d’éditeur reconnu ; empreintes SHA-256 jointes. Aucun pilote installé ou remplacé.

Validation locale : **17/17 tests**, comparaison FFT/DFT, contrôles des niveaux bruts et de la palette, puis réglages restaurés dans des processus séparés. **RTL-SDR réel avant/après redémarrage : 45 s + 30 s**, 659 contrôles de plage fixe réussis, 34 996 224 octets IQ, aucune erreur. Les graduations restent à −20 / −100 dBFS malgré les variations du signal. [Capture réelle après redémarrage](https://github.com/krauschris95-alt/pocsag-sdr-pro/blob/main/docs/qualification-0118/spectrum-rtl-after-restart.png), [rapport complet](https://github.com/krauschris95-alt/pocsag-sdr-pro/blob/main/docs/QUALIFICATION-0.1.18.md).
