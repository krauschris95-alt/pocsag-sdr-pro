# Vecteurs IQ indépendants et non confidentiels

Ces six fichiers sont des vecteurs synthétiques externes : ils ne sont pas des captures radio.
Ils ont été produits par `gen-ng` de multimon-ng, commit
`b5f7d71c5e9b5a76093ba55c515968dc40f92804`, puis validés par le décodeur de cet outil.
Le programme d'encodage interne de POCSAG SDR Pro n'intervient jamais dans leur création.

Le script `scripts/make_reference_fixtures.py` documente et reproduit la génération.
Il remodule la sortie audio externe à ±4500 Hz en CU8 à 240000 éch/s.
Les fichiers `.iqz` utilisent le format qCompress : taille big-endian uint32 puis zlib.
`manifest.json` contient les adresses, fonctions, types et textes attendus.
Les textes sont exclusivement des chaînes de test bénignes choisies pour ces vecteurs.

Provenance de l'outil : https://github.com/EliasOenal/multimon-ng
Licence de la collection et des scripts de ce projet : GPL-3.0-or-later.
Aucun code de multimon-ng n'est inclus dans le moteur de l'application et cet outil
n'est pas nécessaire à la réception ou au fonctionnement du logiciel livré.
