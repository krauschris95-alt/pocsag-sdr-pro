# POCSAG SDR Pro 0.1.5 — Save capcodes et état SDR

La disposition validée des deux tableaux est conservée.

## Cause du plantage Save

Le tableau créait 15 colonnes mais n'initialisait jamais la cellule Voix (colonne 14). rowValues() déréférençait ce pointeur nul lors de Save et de l'export JSON. Reproduction locale avant correction : cellule 0x0, sortie Windows -1073741819 (0xC0000005).

La cellule est désormais initialisée sur ajout, chargement et import, et son accès est vérifié. Save valide toutes les lignes et chemins avant la transaction. Les erreurs conservent le dialogue et les données saisies. SQLite est mis à jour par une transaction annulée en cas d'échec. capcodes-backup.json contient une copie atomique de la configuration précédemment enregistrée, réimportable via Importer JSON.

Le dossier dédié et le fichier TXT direct restent deux choix distincts. Champs modifiables, boutons Parcourir et Ouvrir le dossier ; création des dossiers et fichiers absents, ajout sans écraser un fichier existant. Les chemins refusés ou fichiers verrouillés produisent une erreur explicite.

## État SDR réel

Voyant vert SDR ACTIF seulement si le thread reçoit des IQ matériels récents, indépendamment des messages POCSAG. Rouge SDR INACTIF à l'arrêt, en absence de données ou après perte du flux. Expiration après 1,5 seconde sans IQ, vérification visuelle toutes les 250 ms. Une lecture IQ hors ligne n'active pas le voyant matériel.

Les erreurs Save sont consignées dans technical.log. Les exceptions Qt non interceptées et, au mieux, les exceptions fatales Windows sont consignées dans exceptions.log dans le dossier local de l'application. Ce journal ne remplace pas un dump de diagnostic et ne garantit pas la récupération d'une faute mémoire arbitraire.

Voir VALIDATION.md pour les résultats réellement obtenus, les essais matériels et les limites.
