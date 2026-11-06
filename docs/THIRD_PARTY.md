# Dépendances

Le moteur POCSAG est une implémentation indépendante : aucun code de multimon-ng n'est copié.
La comparaison avec multimon-ng est documentée dans VALIDATION.md.

Qt 6.8.3 est lié dynamiquement (Core, GUI, Widgets, SQL et plugin SQLite), selon les licences
LGPL v3/GPL applicables aux composants. Sources, notices et textes : https://download.qt.io/archive/qt/6.8/6.8.3/
et https://doc.qt.io/qt-6/licensing.html. L'utilisateur peut remplacer les bibliothèques dynamiques.
Le paquet doit conserver les textes de licence fournis avec Qt avant distribution officielle.

MinGW : les DLL de runtime redistribuées relèvent de leurs licences et de l'exception GCC runtime.
Le déploiement est réalisé par windeployqt.

RTL-SDR : bibliothèque chargée dynamiquement, version Blog V1.4.0 incluse dans l'archive.
Les sources exactes correspondantes sont dans `sources/rtl-sdr-blog-V1.4.0.zip`.
Utiliser la distribution officielle RTL-SDR Blog pour V3/V4 : https://github.com/rtlsdrblog/rtl-sdr-blog
Les DLL et leurs dépendances doivent correspondre à l'architecture x64.
Respecter les licences GPL des bibliothèques RTL-SDR si elles sont redistribuées.
Le code original de cette application est fourni sous GPL version 3 ou ultérieure ; voir LICENSE.
HackRF 2026.01.2 et libusb 1.0.30 : DLL MSYS2 MinGW x64 épinglées par SHA256 dans scripts/package_hackrf.py. Licences et métadonnées dans licenses/hackrf et licenses/libusb ; sources correspondantes dans sources/. Runtime libwinpthread déployé avec MinGW. Sources officielles : https://github.com/greatscottgadgets/hackrf et https://github.com/libusb/libusb. SAPI est un composant Windows existant, sans DLL vocale redistribuée.




0.1.6 : les mêmes sources RTL-SDR Blog V1.4.0 sont recompilées avec MinGW et libusb 1.0.30, supprimant les dépendances VCRUNTIME140/MSVCR100 de l’ancienne DLL. Bibliothèque d’import C officielle utilisée pour le module SoapyRTL. Aucun changement de l’algorithme RTL. Reproduction et SHA256 : scripts/build_sdr_modules.py.
SoapySDR 0.8.1 : Boost Software License 1.0, source https://github.com/pothosware/SoapySDR/tree/soapy-sdr-0.8.1 ; SoapyRTLSDR 0.3.3 : MIT, https://github.com/pothosware/SoapyRTLSDR/tree/soapy-rtl-sdr-0.3.3. Sources exactes dans sources/, licences dans licenses/. Les cinq en-têtes C officiels sont conservés dans third_party/SoapySDR avec leur licence. Aucun autre module constructeur redistribué.
NSIS 3.12 : outil de construction de l’installateur, https://nsis.sourceforge.io/License ; compiler non requis sur le PC utilisateur. L’installateur généré contient l’application et ses notices. Les bibliothèques dynamiques restent remplaçables.

Airspy 0.1.17 : libairspy et libairspyhf reconstruites depuis les révisions officielles épinglées dans scripts/build_airspy.py (fc61ab6be57ed61f0e2bdd9c6dfae74cacef57d0 et 24fe8ffcb00b14f827268bbad89ae1392de055e5). Licences BSD conservées dans licenses/airspy et licenses/airspyhf, archives source exactes dans sources/. Liées au même libusb x64 que les autres adaptateurs. Aucune DLL SDRplay, LimeSuite, bladeRF ou libiio redistribuée dans cette version. L’adaptateur FUNcube utilise les API système Windows existantes. Les simulateurs tests/mock_*.cpp sont des bancs isolés et sont exclus du paquet.
