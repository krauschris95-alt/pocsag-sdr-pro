# Dépendances

Le moteur POCSAG est une implémentation indépendante : aucun code de multimon-ng n'est copié.
La comparaison avec multimon-ng reste à réaliser sur des captures identifiées.

Qt 6.8.3 est lié dynamiquement (Core, GUI, Widgets, SQL et plugin SQLite), selon les licences
LGPL v3/GPL applicables aux composants. Sources, notices et textes : https://download.qt.io/archive/qt/6.8/6.8.3/
et https://doc.qt.io/qt-6/licensing.html. L'utilisateur peut remplacer les bibliothèques dynamiques.
Le paquet doit conserver les textes de licence fournis avec Qt avant distribution officielle.

MinGW : les DLL de runtime redistribuées relèvent de leurs licences et de l'exception GCC runtime.
Le déploiement est réalisé par windeployqt.

RTL-SDR : bibliothèque chargée dynamiquement, non incluse dans la première archive.
Utiliser la distribution officielle RTL-SDR Blog pour V3/V4 : https://github.com/rtlsdrblog/rtl-sdr-blog
Les DLL et leurs dépendances doivent correspondre à l'architecture x64.
Respecter les licences GPL des bibliothèques RTL-SDR si elles sont redistribuées.
HackRF : non intégré dans la version 0.1.0.
