# Moteur FLEX externe — attribution et licence

Cette branche de développement intègre un **moteur FLEX réel** en réception seule, construit depuis le projet libre [multimon-ng](https://github.com/EliasOenal/multimon-ng), révision épinglée `10c0aeb68bc78a58b7019a45ab4e1bfee4fc9592` (15 novembre 2026).

Le fichier `multimon-ng.exe` est compilé séparément puis livré à côté de `POCSAG-SDR-Pro.exe`. Le code multimon-ng est couvert par des licences GPL indiquées dans ses fichiers sources (généralement GPL-2.0-or-later et/ou GPL-3.0-or-later). La distribution inclut sa licence `COPYING` et le lien vers les sources exactes. Respecter les attributions et obligations de redistribution.

Le projet POCSAG SDR Pro utilise son propre décodeur POCSAG inchangé. Le flux IQ CU8 est filtré/démodulé et converti en PCM 16 bits 22 050 Hz pour `FLEX_NEXT`. Seules les pages JSON conformes aux champs attendus et sans indicateur d'intégrité négatif sont proposées à l'interface FLEX.

**Limites** : ce pont n'est pas une qualification RF réelle ; ses performances de réception, les variantes FLEX et la stabilité 24 h doivent être testées avec captures IQ indépendantes. Les messages FLEX sont conservés dans une table SQLite séparée afin d'éviter de confondre les adresses FLEX (jusqu'à 4 297 068 542) avec les capcodes POCSAG (21 bits). La voix et le Pager PrivateRadio ne sont pas activés pour FLEX sans règles de filtrage explicites.

**ReFLEX** : non décodé par multimon-ng FLEX_NEXT. Cette branche ne doit pas annoncer la compatibilité ReFLEX. Le choix manuel ReFLEX est désactivé tant qu'un vrai décodeur et des preuves de réception ne sont pas disponibles.

Ne pas fusionner dans la version de production tant que les tests CI, les échantillons FLEX et les régressions POCSAG ne sont pas qualifiés.
