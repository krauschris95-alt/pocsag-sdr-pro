# Récepteurs, pilotes et limites — 0.1.17

La présence dans « Interfaces Windows » prouve seulement que Windows voit une interface USB. Une ligne dans « Récepteurs détectés par les API » indique une énumération du backend. Seul « Tester IQ » ou une réception active avec échantillons permet de vérifier le flux. Le voyant utilise l’arrivée récente des IQ, même en l’absence de message POCSAG. Les essais simulés ne qualifient ni le câble, ni le pilote, ni le tuner d’un appareil physique.

| Famille | Chemin de réception Windows | Dépendances et état de qualification |
| --- | --- | --- |
| RTL-SDR, Blog V3/V4 | API native RTL Blog V1.4.0 fournie ; module SoapyRTL fourni | WinUSB/libusb approprié. Blog V4 physique testé ; V3 et autres tuners non éprouvés dans cette livraison. |
| HackRF One, PortaPack | libhackrf x64 fournie, IQ signés CS8 convertis vers CU8 | Mode USB HackRF requis ; le firmware autonome PortaPack n’est pas un flux USB IQ. Pilote USB approprié. Simulation du transport, décodage de référence et reconnexion ; matériel indisponible aux outils pour qualification. |
| Airspy R2 / Mini | libairspy x64 fournie, CF32 | API officielle, pilote USB compatible avec libusb. Gain de linéarité : indice 0–21, pas d’unité dB inventée. AGC LNA/mélangeur. Taux interrogés sur l’appareil. Modèle précis non fourni par l’énumération : ne pas confondre R2/Mini sur la seule série. Tests ABI simulés et signal indépendant ; aucun Airspy physique qualifié. |
| Airspy HF+ | libairspyhf x64 fournie, CF32 | API officielle, taux et présence AGC interrogés. PPM logiciel. Pas de faux réglages de gain, antenne ou bande. Même limite de qualification matérielle. |
| SDRplay RSP | SoapySDRPlay3 + API SDRplay constructeur | **Module et SDK constructeur non fournis.** API/service/pilote constructeur requis. Le module officiel demande une API adaptée à sa version. Ne jamais remplacer le pilote SDRplay par WinUSB. Pas de qualification matérielle ici. |
| FUNcube Pro / Pro+ | Adaptateur natif expérimental HID + WinMM PCM16 stéréo | Conserver HID et USB Audio Windows. Association stricte par ContainerId, jamais le microphone par défaut. Formats directs 48/96/192 kéch/s interrogés, commande fréquence et relecture HID vérifiées. Protocole/PCM et décodage simulés ; association HID/audio et tuner à éprouver physiquement. Gain/AGC/antenne/bande non exposés par cet adaptateur. |
| LimeSDR | SoapyLMS7 + LimeSuite Windows | Module/SDK compatibles à fournir, non redistribués ici. |
| bladeRF | SoapyBladeRF + libbladeRF | Même exigence de versions et architecture. |
| PlutoSDR USB/réseau | SoapyPlutoSDR + libiio/transport approprié | La connexion doit être annoncée par le module ; l’inventaire USB n’énumère pas les appareils réseau. |
| Autres | Module Soapy RX compatible | Aucun support universel annoncé. Vérifier format CF32, capacités, taux et réception réelle. |

Les simulations Soapy des libellés SDRplay/Lime/bladeRF/Pluto vérifient le contrat C commun, les erreurs et le décodage ; elles n’exécutent pas ces quatre modules constructeurs et ne les qualifient pas.

## Diagnostic et choix du runtime

Dans **Récepteur → SDR détectés et pilotes…**, consulter les trois premiers onglets : séries, backend, identifiant USB, service du pilote Windows, problèmes de périphérique, présence/chargement des DLL, dossiers de modules et erreurs de chargement disponibles. Le service interrogé pour SDRplay est `SDRplayAPIService`, pas le service Windows `SDRSVC`.

Si aucun SDR n’est ouvert par son API alors qu’une interface USB apparaît, résoudre le pilote/API manquant avec le fabricant. L’application ne lance aucune installation, ne change aucun pilote et ne désactive aucune protection Windows. Ne pas appliquer un changement WinUSB à toutes les interfaces d’un périphérique composite, particulièrement FUNcube et SDRplay.

Pour un SDK Soapy déjà installé, l’onglet **Runtime Soapy externe** mémorise la DLL x64 ABI 0.8 et le dossier de ses modules, puis demande un redémarrage. Choisir un ensemble cohérent du même compilateur/SDK : un module MSVC ne doit pas être mélangé avec le runtime MinGW fourni. Les DLL dépendantes des modules doivent également être accessibles selon la distribution du fabricant. L’API C relie l’application au runtime ; les modules C++ restent liés à leur propre runtime. Un chemin explicite non chargeable provoque un diagnostic, sans remplacement silencieux par la DLL fournie. Les variables d’environnement explicitement définies restent prioritaires.

La configuration ne remplace pas l’installation officielle de l’API SDRplay et de son service. Fermer les autres logiciels qui utilisent le même SDR avant un essai. Le bouton Tester observe les IQ pendant cinq secondes dans une session sans écrire les messages dans les journaux ou la base utilisateur. Il refuse une seconde ouverture du récepteur déjà actif.

## Identités et limites communes

Les clés utilisent la série native, les arguments Soapy ou l’association HID/audio FUNcube. Les doublons native/Soapy de même famille et série sont fusionnés ; une série native partagée est désactivée. Un RTL sans série lisible reste un diagnostic non sélectionnable. Si la sélection mémorisée est absente, elle reste indiquée comme absente au lieu de basculer vers une autre clé. Un FUNcube sans série USB peut nécessiter une nouvelle sélection après changement de port : aucune substitution audio automatique.

Taux communs acceptés : 48 kéch/s à 20 Méch/s, sous réserve du pilote, du tuner, du débit USB et du CPU. HackRF impose au moins 2 Méch/s. Les réglages non annoncés sont désactivés. La fréquence reste limitée par le stockage 32 bits à 4 294,967295 MHz ; les limites du matériel peuvent être plus basses ou disjointes. La conversion finale vers CU8 conserve le moteur POCSAG existant. Aucun algorithme de démodulation, BCH, interprétation ou filtrage n’a été remplacé pour ce changement.

Plusieurs sessions sont déjà préparées par « Récepteurs supplémentaires », mais seule la séparation d’identités est testée ici ; plusieurs récepteurs physiques simultanés ne sont pas qualifiés. Un appel bloquant dans une DLL constructeur peut retarder l’arrêt. Windows 10 et Windows 11 doivent encore être qualifiés séparément sur les matériels concernés ; une compilation x64 et un essai sur un poste ne suffisent pas à annoncer toute cette matrice.

Sources officielles : [Airspy](https://github.com/airspy/airspyone_host), [Airspy HF+](https://github.com/airspy/airspyhf), [HackRF](https://github.com/greatscottgadgets/hackrf), [SDRplay API](https://www.sdrplay.com/hardware-api/), [SoapySDRPlay3](https://github.com/pothosware/SoapySDRPlay3), [FUNcube](https://www.funcubedongle.com/), [WinMM](https://learn.microsoft.com/en-us/windows/win32/api/mmeapi/nf-mmeapi-waveinopen). Le module SoapyFCDPP amont utilise ALSA et n’est pas présenté comme un module Windows dans ce paquet ; le nouvel adaptateur Windows est distinct.
