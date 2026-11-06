# Qualification 0.1.18 — spectre et waterfall

## Périmètre

Le changement porte sur le widget Spectrum, ses réglages et les libellés de version/distribution. Les sources du décodeur, de l’ingestion, des capcodes, des journaux, du stockage SQLite, de la voix et de PrivateRadio ne sont pas modifiées. L’historique et les fichiers utilisateur sont préservés. Les essais de réception utilisent un dossier de travail isolé ; aucun corps de message radio, IQ ou base de travail n’est publié.

L’ancien défaut était −120 dBFS en bas et la boucle des graduations n’affichait pas la borne inférieure. Le nouveau mode fixe utilise exactement −100 / −20 dBFS et neuf graduations de 10 dB, dans une marge distincte de la courbe et du waterfall. Le mode automatique, facultatif, ajuste seulement les limites de représentation selon la FFT ; il ne modifie pas les mesures.

## FFT et affichage

Transformée complexe 1 024 points, fenêtre de Hann et normalisation d’amplitude `/512` conservées depuis 0.1.17 : `20 log10(max(1e-8, abs(FFT)/512))`. Aucun décalage ajouté au bruit, aucune normalisation de la courbe sur un niveau cible. Les valeurs brutes sont conservées séparément de la courbe lissée. Le lissage optionnel est une moyenne exponentielle de puissance, coefficient 1/N (1 à 30) ; N = 1 le désactive. Le waterfall utilise les niveaux bruts, avec une palette bleu nuit → azur → cyan glacé. Contraste et luminosité changent uniquement la correspondance couleur.

Les neuf repères sont −20, −30, −40, −50, −60, −70, −80, −90 et −100 dBFS. Les réglages de mode, contraste, luminosité, lissage et vitesse waterfall sont enregistrés immédiatement dans le dossier de configuration existant, sans déplacer la base ou les journaux. Le changement de fréquence réinitialise seulement l’historique graphique, comme auparavant. Le repère rouge et le clic d’accord utilisent désormais la zone du tracé, en tenant compte de la marge des graduations.

## Validation

Les preuves sont consignées dans `qualification-0118/` : comparaison avec une DFT indépendante, absence de modification des valeurs brutes par les contrôles visuels, maintien des bornes sur des signaux de puissances différentes, mode automatique facultatif et restauration fixe, palette monotone bleue, persistance des réglages, puis deux processus de réception RTL successifs utilisant la même configuration de test. Les captures de la fenêtre Qt compilée proviennent de vrais IQ RTL, pas d’un spectre simulé. Les captures synthétiques et les essais matériels sont distingués dans les rapports.

La qualification physique est limitée au Blog V4 disponible à 169,626 MHz et au poste Windows utilisé. Les autres familles SDR gardent les limites de qualification de 0.1.17. Les exécutables restent non signés ; aucune protection Windows désactivée.

### État local avant qualification matérielle

Compilation Windows x64 réussie. **17/17 suites locales passent en 79,97 s** (`ctest-final.log`). Le test spectre a ensuite été renforcé avec fermeture et relancement dans **deux processus Windows séparés** : mode fixe, bornes −100 / −20, contraste 1,3, luminosité 8 % et lissage 6 restaurés ; contrôle réussi (`spectrum-restart-process.log`). La comparaison DFT et l’absence de modification des niveaux bruts passent également.

La première tentative matérielle ne reçoit aucun IQ : Windows voit le Blog V4 série 00000001, WinUSB et problème 0, mais `rtlsdr_get_device_usb_strings(0)` échoue et l’ouverture par série n’est pas possible. Cette tentative n’est pas une validation RTL et ses images sans IQ ne sont pas fournies comme preuve. La réception d’une autre application doit être arrêtée, ou la clé rendue disponible, avant les deux sessions réelles de confirmation. **Aucune publication de 0.1.18 avant cette validation.** Cette première tentative a été conservée dans le cache privé de travail, pas comme preuve de réception.

### Réception réelle et redémarrage validés

Après libération du RTL-SDR par l’utilisateur, **les deux sessions réussissent**, à 169,626 MHz sur le Blog V4 : 45 secondes avant fermeture, puis 30 secondes dans un nouveau processus utilisant le même dossier de test. Aucun code applicatif modifié entre les sessions. Les 659 contrôles pendant l’arrivée effective d’IQ trouvent tous la plage fixe −100 / −20 dBFS ; les neuf graduations sont identiques. Contraste 1,3, luminosité 8 % et lissage 6 sont restaurés après redémarrage.

Total : 34 996 224 octets IQ et 420 trames du décodeur, aucune erreur de flux. Rapports `rtl-before-restart.json` et `rtl-after-restart.json`. Captures `spectrum-rtl-before-restart.png` et `spectrum-rtl-after-restart.png`, plus les deux fenêtres de réglages. Les captures ont été inspectées : −20 dBFS visible en haut, −100 dBFS visible sur la dernière graduation, cyan lisible et waterfall bleu. Le maximum FFT brut varie de −46,96 à −6,18 dBFS entre les instantanés, sans déplacer l’échelle. Une crête au-dessus de −20 est simplement coupée au bord du tracé fixe ; sa valeur brute n’est ni abaissée ni recalée artificiellement. Le waterfall est un historique de session et se remplit progressivement après chaque démarrage.

La restriction de publication liée à l’essai réel est donc levée : réception et restauration après redémarrage sont maintenant vérifiées. Les bases et journaux de l’utilisateur n’ont pas été ouverts par ce banc ; son dossier contient uniquement des données de test isolées, exclues du paquet.

### Distribution et GitHub Actions

GitHub Actions [run 42211859108](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/42211859108), commit 13cee84 : compilation Windows x64, **17/17 suites réussies**, déploiement et artefact réussis (6 min 38 s). Artefact 13499991598, digest SHA-256 `4c062cf66bcce3a957bc7d019cc6d7e04038d6fbaad3f4fa837342aab3ad7066`. Sortie des tests conservée dans `github-tests.txt`.

Les paquets locaux livrés sont qualifiés séparément : ZIP extrait, CRC et empreintes des 252 fichiers vérifiés ; lancement sans Qt dans PATH ; essai RTL supplémentaire de 10 s (540 672 octets pour le spectre, sans erreur). Installateur : installation, lancement et désinstallation isolés réussis, 252 fichiers identiques, fichier TXT témoin préservé. Rapports `installer-test.json`, `portable-rtl.json`, `release-assets.json`. Ces archives locales ne sont pas présentées comme identiques octet par octet à l’artefact CI.

ZIP : SHA-256 `9bf908a6440dfc48eaefb40bf84ff12dd1f67d3a38b3f1b78a128eed72b5163a`.
Installateur : SHA-256 `9811c173c19f298e6467bfab79827b90878da36fe4d62f672fadf7a02fbac72b`.

Les derniers ajouts de qualification sont documentaires ; ils ne changent aucun code applicatif depuis les essais matériels et CI. Livraison prévue sur la [préversion v0.1.18](https://github.com/krauschris95-alt/pocsag-sdr-pro/releases/tag/v0.1.18).
