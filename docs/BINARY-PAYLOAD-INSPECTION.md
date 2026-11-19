# Analyse des charges utiles binaires et Base64 (branche de travail)

## Pourquoi des messages restent illisibles

Une chaîne Base64 est un **encodage d'octets**, pas un algorithme de chiffrement. Des octets non textuels peuvent représenter un format applicatif, des données compressées, une transmission mal interprétée ou un contenu chiffré. La forme Base64 seule ne permet d'identifier ni ASTRID, ni AES, ni IDEA, ni une clé. Les fichiers de qualification 0.1.13 ont établi que les mêmes chaînes apparaissaient dans d'anciens décodeurs et dans multimon-ng, sans preuve d'un algorithme de chiffrement particulier.

## Ajout proposé

Dans le double-clic **Inspection du message**, pour un texte reconnu comme Base64 canonique par l'analyseur existant :

- nombre total d'octets décodés ;
- aperçu hexadécimal et ASCII (64 premiers octets, sans interprétation arbitraire) ;
- indication de signature si elle correspond à gzip, ZIP, PNG, JPEG ou PDF ; sinon « format binaire inconnu » ;
- entropie de Shannon indicative, explicitement **non probante pour le chiffrement** ;
- avertissement indiquant qu'aucun déchiffrement n'a été effectué.

L'inspection est **locale, en lecture seule**. Aucun message d'origine, bit radio, capcode, historique SQLite, journal TXT, manifeste JSON, synthèse vocale ou WAV n'est réécrit. Aucun message privé n'est publié ni envoyé à un service réseau.

## Limites

- La reconnaissance Base64 demeure volontairement prudente (longueur minimale, alphabet, padding et bits inutilisés canoniques) : les petites chaînes ou encodages personnalisés peuvent ne pas être reconnus.
- Les signatures détectent seulement un préfixe plausible ; elles ne prouvent pas la validité d'un fichier complet. Aucune décompression n'est exécutée.
- Un score d'entropie élevé ne prouve pas un chiffrement, en particulier sur un message court.
- Sans spécification du format, captures IQ correspondantes et, si nécessaire, **clé de déchiffrement obtenue légalement**, aucun texte clair ne peut être reconstruit de manière fiable. Aucune tentative de contournement, récupération ou devinette de clé.
- Le décodeur FLEX/ReFLEX est un chantier **séparé**. Il ne faut pas confondre format de radiomessagerie et encodage/chiffrement du contenu applicatif.

## Validation

Le test `binary_payload_inspection` couvre les octets Base64 canoniques, taille et aperçu exacts, troncature, ASCII, entropie, signatures, caractères invalides, padding non canonique et préservation du classement existant. La compilation et les tests Windows doivent passer avant toute fusion. Aucune nouvelle release n'est autorisée par cette branche.
