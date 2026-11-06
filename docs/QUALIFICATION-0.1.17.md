# Qualification 0.1.17

## Périmètre et séparation des preuves

La version ajoute les adaptateurs et le diagnostic matériel sans modifier les sources du décodeur POCSAG ni les deux tableaux. Les essais utilisent des dossiers temporaires et des messages synthétiques. Aucun fichier SQLite, TXT, WAV ou configuration de l’utilisateur n’est déplacé, modifié ou publié. Le serveur officiel PrivateRadio n’est pas contacté.

Le poste d’exécution indique Windows 11 Version 26H2 via Qt et Windows NT 10.0.26300, architecture x64, Qt 6.8.3 et GCC 13.1.0. Ce résultat ne remplace pas une qualification séparée Windows 10/11. L’inventaire USB accessible aux outils voit un Blog V4, série 00000001, VID 0BDA/PID 2838, WinUSB, code de problème 0. Les familles indiquées par l’utilisateur sont Airspy, HackRF et SDRplay ; elles ne sont pas physiquement qualifiées par l’inventaire disponible.

## Résultats

Les résultats finaux de compilation, CTest, distribution et contrôles visuels sont consignés dans `qualification-0117/`. Les essais automatisés couvrent notamment : ABI Airspy/HF+, deux séries par famille, conversion CF32/PCM16, signal externe multimon-ng, valeurs extrêmes/non finies, DLL absente, taux refusé, accord refusé, coupure ; protocole FUNcube, accusé HID, relecture de fréquence et tampon PCM tronqué ; contrat C Soapy avec quatre familles simulées, ABI incompatible et erreurs de flux ; interface SDR, deux thèmes, configuration du runtime et absence de substitution de la sélection mémorisée. Les mocks ne sont jamais distribués comme pilotes.

Le premier essai réel Blog V4 dure 93 s : 43 270 144 octets IQ, 92 trames du décodeur, 12 lignes SQLite au total dont trois messages de test synthétiques. Changements 169,626 → 169,625 → 169,626 MHz en 1 608 et 1 612 ms, IQ actifs sur les trois périodes. Deux TXT simultanés exacts, trois annonces SAPI terminées et trois WAV valides, arrivée IQ pendant les annonces, fenêtre réduite. Voir `qualification-0117/rtl-live.json`. Les corps de messages radio et fichiers de travail temporaires ne sont pas publiés.

La suite existante vérifie également les débits 512/1200/2400, ALPHA/NUMERIC/TONE/AUTO, BCH, verrouillage SQLite et reprises, journalisation indépendante et changements de date, capcodes, voix/JSON/WAV, fréquence/favoris et thèmes. Les nouveaux tests ne prétendent pas éprouver les modules constructeur SDRplay/Lime/bladeRF/Pluto eux-mêmes : seule l’interface C commune est simulée pour ces libellés.

Seize suites CTest finales : 16/16, 87,37 s (`ctest-final.log`). HackRF signé CS8 : trame externe décodée avant et après coupure simulée, série et modèle de l’API conservés, voyant rouge pendant la coupure puis reprise ; le premier banc interrompait avant la fin du signal et a été corrigé pour déclencher la coupure après une trame complète. Soapy RTL physique : 30 s à 250 000 éch/s, neuf textes, aucune erreur, IQ actifs et voyant rouge après arrêt (`soapy-rtl-live.json`). Contrôles de la fenêtre sur les deux thèmes à facteurs Qt 100/125/150 % ; bouton Capacités puis Tester IQ vérifié sur le RTL physique, sans insertion de messages dans la base temporaire principale. La lecture de série USB peut être refusée si un autre processus détient déjà la clé ; les captures finales ont été refaites après libération du flux.

## Limites

Pas de qualification physique Airspy, HackRF, SDRplay ou FUNcube, ni de plusieurs récepteurs physiques simultanés. L’association des interfaces HID/audio FUNcube reste expérimentale jusqu’à un essai matériel. Aucun SDK SDRplay installé détecté ; son module n’est pas redistribué. Modules supplémentaires et SDK requis selon la matrice `SDR-COMPATIBILITY.md`. Pas de certification sur Windows 10 séparé. Aucun certificat d’éditeur disponible : EXE/installateur non signés, procédure préparée et empreintes publiées. Les changements de facteurs Qt ne prouvent pas plusieurs écrans/DPI système physiques.
