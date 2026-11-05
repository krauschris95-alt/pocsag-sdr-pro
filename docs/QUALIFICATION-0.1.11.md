# Qualification POCSAG SDR Pro 0.1.11

Base 0.1.10 : c4c1b46bd68067b16b3d57a49e41dcd0827aa75f. Code applicatif final : b629f873dbf8610ea9b68ba28fadd6d91a86be42. Premier essai radio : cc51fcdfaa03bdc2e5c6eeed70738afc4232716e. Décodeur, démodulation, récepteurs, format/routage TXT, profils et disposition des deux tableaux conservés. Ajout transactionnel Pager et diagnostics sans texte privé.

## Voix

Défaut déterministe reproduit dans une base isolée : la déduplication vocale utilisait le numéro de ligne SQLite. Après suppression d’historique et réutilisation de ce numéro, un nouvel événement était refusé comme doublon. La voix utilise désormais l’UUID de réception, comme la file Pager. Cette cause est prouvée par test, mais n’est pas attribuée sans preuve à la configuration utilisateur.

Réouverture de ISpVoice et sortie sur le thread vocal avant chaque annonce, secours français si jeton disparu, repli signalé sur sortie par défaut si sortie configurée absente. Surveillance de l’identité du périphérique par défaut pendant la lecture ; erreur ou changement entraîne une seule nouvelle tentative réinitialisée. Une phrase interrompue peut recommencer ; aucune reprise au mot exact promise.

XML échappé, pause de 300 ms. Diagnostics : éligibilité, refus de file, début/fin, coupure, volume Windows, périphérique absent/muet, erreurs COM. Pas de texte d’intervention. Dialogue avec chemin réel de base/préférences et interrupteurs enregistrés. Rétablissement français/sortie par défaut sans modifier les réglages audio globaux Windows.

La cause unique du défaut rapporté sur les interventions réelles n’est pas prouvée. Une lecture initiale d’un fichier de base actif a échoué, puis le contrôle rapide après fermeture a indiqué ok mais un schéma différent de la configuration vocale attendue. Aucune réparation, remplacement ou modification de cette base. Aucune corruption définitive revendiquée.

## Audio PrivateRadio

Tâche durable dans la transaction du message pour Actif + Pager ON, indépendante de Voix/TXT/En bas. WAV SAPI direct PCM 16 bits mono 22 050 Hz, noms UUID, contrôle RIFF/hash, manifeste JSON après publication atomique du WAV. Nom neuf dans sous-dossier temporaire du volume cible : une réservation QTemporaryFile était incompatible avec BindToFile et renvoyait 0x80070057, détecté et corrigé sur SAPI réel.

Persistance, expiration, tentatives, reprises, fichiers absents/incomplets et accusés. Aucun fichier invalide existant effacé ou écrasé automatiquement. Format final, priorités SOS/PTT, canal Pager, comptes et clients restent à intégrer dans l’espace serveur séparé ; aucun serveur officiel contacté.

## Résultats obtenus

- 10/10 suites CTest finales en 58,48 secondes : régressions décodage indépendant, SQLite, TXT/minuit, tableaux ; combinaisons Voix/Pager, WAV/JSON, dossier inaccessible, reprise/redémarrage, expiration, fichiers incomplets/manquants et reconnexion audio injectée.
- Sortie Windows réelle : messages synthétiques via Window::received, SQLite et file automatique, fenêtre réduite. Trois annonces terminées et signal WASAPI mesuré ; sortie non muette, volume 59 %. Confirmation utilisateur des alias CIP112, R-MED1, R-MED2 puis messages audibles. Aucun bouton Tester utilisé.
- Quatre WAV SAPI prêts pour trois annonces locales : quatrième tâche Pager ON / Voix OFF. windows-automatic-audio.json. Fichiers temporaires synthétiques ; aucune capture microphone ni fabrication des WAV depuis les haut-parleurs.
- HTTPS LAB sur 127.0.0.1/port éphémère : certificat non approuvé et mauvais jeton refusés ; événement + octets reçus, chemin local retiré, répétition dédupliquée et accusé stable. Aucun changement de production, Caddy, Tailscale ou permissions. https-lab.json.
- Dialogues Windows dossier audio et voix validés ; captures synthétiques sous qualification-0111/captures.

## Limites

Windows 11 / Qt 6.8.3 / MinGW 13.1 x64. Windows 10 non essayé. Déconnexion physique et changement physique de sortie non réalisés ; exception et réinitialisation testées par injection. Aucun support externe ou partage autorisé pour qualification dédiée. L’audibilité confirmée vaut pour le PC essayé ; périphérique muet/absent ou autre configuration à diagnostiquer localement.

Serveur et clients non intégrés. Le LAB prouve HTTPS strict et accusés du contrat d’essai, pas une API PrivateRadio encore non étudiée. Manifestes = instantanés de publication ; SQLite = état de transfert. ACL et autorisation des comptes consommateurs à configurer explicitement.

C: était presque plein. Seules des distributions locales retéléchargeables et une bibliothèque d’analyse Python du cache de développement ont été retirées pour compiler. Aucune base, TXT utilisateur ou IQ privé supprimé. Génération différée sous 16 Mio libres, tâches conservées ; pas de suppression automatique.



## Essais radio réels

Essai initial de dix minutes : 600 secondes, 287375360 octets IQ, 3137 trames avant filtrage, 275 textes acceptés dans chaque tableau, 825 lignes dans 5 TXT comparés octet pour octet à l’outbox. 5 verrous SQLite injectés, 153 reprises ; aucun TXT en attente, aucun poll IQ manqué. Trois annonces SAPI terminées, trois WAV prêts, IQ continuant pendant la voix, fenêtre réduite. Base/TXT temporaires isolés ; seuls les messages synthétiques de qualification sont autorisés pour Voix et Pager.

Essai sur le code final, cinq minutes : 300 secondes, 143392768 octets IQ, 1639 trames avant filtrage, 171 textes acceptés dans chaque tableau, 513 lignes dans 5 TXT comparés octet pour octet à l’outbox. 3 verrous SQLite injectés, 97 reprises ; aucun TXT en attente, aucun poll IQ manqué. Trois annonces SAPI terminées, trois WAV prêts, IQ continuant pendant la voix, fenêtre réduite. Base/TXT temporaires isolés ; seuls les messages synthétiques de qualification sont autorisés pour Voix et Pager.

Les essais prouvent la continuité après acceptation et la correspondance des sorties ; ils ne démontrent pas la réception de toutes les émissions de l’émetteur. Aucun texte ou audio radio privé dans les preuves.

## Sources techniques

[Microsoft — ISpStream::BindToFile](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ms719484(v=vs.85)) et [ISpMMSysAudio](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee450831(v=vs.85)). Les résultats acoustiques et WAV proviennent des tests du code, pas d’une simple référence documentaire.

## Distribution et provenance

ZIP CRC vérifié, extraction neuve puis lancement avec PATH Windows uniquement : code 0, données temporaires isolées. Installateur : premier essai insuffisant en espace de travail, deux fichiers partiels et désinstalleur absent ; aucune installation utilisateur touchée. Les seuls fichiers de cet essai, identifiés par la liste du paquet, ont été retirés puis l’essai refait après libération d’espace. Installation code 0, tous les fichiers comparés par hash au paquet, lancement code 0. Désinstallation code 0, application retirée et TXT témoin conservé. Le ZIP recréé pour gérer l’espace disque est octet pour octet identique à celui validé. Détails dans portable-test.json, installer-test.json, release-assets.json.

Les distributions sont compilées localement sous Windows 11 / Qt 6.8.3 / MinGW 13.1 x64. Le commit distant a été vérifié via Git et le connecteur GitHub. Une demande de lancement manuel Actions a été effectuée ; aucune nouvelle exécution ni aucun artefact Actions 0.1.11 n’est confirmé dans les résultats visibles au moment de l’assemblage. Les dernières exécutions historiques étaient bloquées par le budget ; cette cause n’est pas présentée comme le résultat d’un nouveau job inexistant. Aucun changement de facturation. La livraison ne revendique pas une compilation Actions.

Les documents inclus dans les archives sont figés avant les essais d’installation ; le rapport du tag ajoute les résultats après assemblage. Aucun audio d’intervention, fichier SQLite, TXT utilisateur ou IQ privé dans les archives.
