# Qualification 0.1.7 — verrouillage SQLite et deux journaux

## Cause reproduite avant réparation

`tests/sqlite_lock_reproduction.cpp` crée une base temporaire WAL de 1000 messages. Un QSqlQueryModel ne lit que ses 256 premières lignes et conserve son curseur. Une seconde connexion écrit, comme le thread TXT. L’insertion sur la connexion de réception échoue ensuite avec **code étendu SQLite 517 / SQLITE_BUSY_SNAPSHOT**, texte exact `database is locked Unable to fetch row`. Après libération du curseur elle réussit. Preuve : `qualification-017/sqlite-reproduction.log`.

Ce conflit de vue WAL n’est pas résolu par un simple busy_timeout. La version 0.1.6 abandonnait le message après l’échec d’insertion. La vidéo utilisateur (16,77 s) a été examinée localement ; ses images et ses éventuels textes radio ne sont pas publiés. L’exception précise vient de la consigne utilisateur et de notre reproduction, sans prétendre lire les détails flous de la vidéo.

## Correctif

- Modèles détachés : requêtes courtes, données chargées par pages, aucun curseur SQLite conservé entre les affichages. Cache maximal 4096 lignes, sans plafond d’historique. Défilement chronologique ascendant et suivi seulement depuis le bas.
- Insertion avec `BEGIN IMMEDIATE`, requêtes terminées explicitement, transaction courte pour message et ses destinations. Échec temporaire : reprise périodique, sans abandon silencieux.
- File de réception atomique locale avant SQLite, relue au redémarrage. Identifiant interne unique dans SQLite, aucune réinsertion après commit suivi d’un arrêt avant retrait de la file.
- TXT hors thread d’interface, requêtes courtes avec reprise, reçus locaux d’offset pour distinguer « déjà écrit mais accusé SQLite en échec » et « nouveau message identique ». Deux événements identiques restent deux lignes.
- Deux destinations indépendantes : tous les messages par jour belge dans le dossier choisi ; capcodes actifs/Journal TXT dans un fichier permanent. Format CAPCODE DATE HEURE MESSAGE, anciens contenus inchangés. Les anciens chemins/profils sont conservés comme métadonnées mais ne routent pas les nouveaux messages.
- Aucune suppression automatique de l’historique, aucun compteur dans l’interface. Mesure réelle de qualité BCH/parité ; aucune valeur inventée lorsque la mesure manque.

## Tests automatisés

Huit suites CTest passent en **23,78 secondes** ; journal final `qualification-017/ctest.log`. Décodeur, références indépendantes 512/1200/2400, files vocales, reconnexion injectée, Save des capcodes et intégration restent testés.

La régression de stockage vérifie notamment : 12 messages pendant un verrou d’écriture externe, conservation des fichiers de réception, reprise intégrale et exacte dans les deux TXT, commit concurrent pendant consultation d’une page, checkpoint WAL sans lecteur retenu, redémarrage avec message en attente, rejeu du même événement après commit sans doublon, échec répété d’accusé TXT après écriture puis reprise, événements différents au contenu strictement identique, quatre combinaisons ON/OFF, capcode non enregistré exclu du permanent, minuit belge, désactivation TXT individuelle, dossier inaccessible puis rétabli, chemins Windows invalides/fichier verrouillé, conservation d’un contenu préexistant et chargement de plus de 100000 messages sans suppression. Le cache reste borné ; les lignes les plus anciennes et les plus récentes demeurent accessibles.

## Essai matériel final

**Réussi : 1380 secondes (23 minutes), dont 1200 secondes avec les deux journaux.** Rapport `qualification-017/hardware-soak.json`, progression dans `hardware-soak.log`.

| Phase | Durée | Textes acceptés | Résultat |
|---|---:|---:|---|
| TXT OFF | 60 s | 4 | SQLite et deux tableaux actifs, aucune destination TXT |
| Général seul | 60 s | 4 | Ajout quotidien uniquement |
| Capcodes seuls | 60 s | 13 | Ajout permanent uniquement |
| Deux journaux | 1200 s | 439 | SQLite, deux tableaux et deux fichiers simultanés, reprises sous verrous |

Total : **460 messages SQLite, 460 lignes dans chaque tableau**. Le journal général contient **443 lignes** (4 + 439), le permanent **452 lignes** (13 + 439). Comparaison finale des octets complets de chaque TXT avec les lignes attendues de la file : **exacte**, sans manquant ni doublon. La différence entre les totaux des fichiers est intentionnelle et correspond aux interrupteurs des phases.

**10 verrous d’écriture de six secondes**, déclenchés sur de vraies trames, ont entraîné **315 reprises SQLite**. La réception IQ et le décodeur continuent pendant le verrou ; SQLite, affichage et TXT rattrapent les messages après sa levée. À la fin : **0 message en attente, 0 TXT en attente, 0 relevé d’IQ périmé** après la phase de connexion, et voyant rouge à l’arrêt. 661995520 octets IQ, 5813 trames de décodeur (incluant types filtrés), 16053 mots mesurés et aucun rejet de mot observé. Ces mesures ne prouvent pas la réception de toutes les transmissions RF ni une qualité radio universelle.

Un contrôle indépendant de la base temporaire en lecture seule pendant la réception a aussi confirmé la croissance de SQLite et des deux fichiers, sans TXT en attente : `live-pipeline-snapshot.json`.

L’essai utilise le même code `Window`, les modèles, le récepteur, le décodeur et le worker TXT, compilés une seule fois dans `desktop_window` puis liés à l’application et à la sonde d’assertions. La sonde ajoute uniquement les contrôles et la configuration isolée ; elle n’est pas un autre moteur. Empreintes dans `build-payload.json`.

 Les tests utilisent une base et des fichiers temporaires isolés ; aucune base ou journal utilisateur n’est modifié. Les capcodes reçus sont enregistrés automatiquement uniquement dans cette sonde de test pour exercer les deux destinations ; ce n’est pas une fonctionnalité d’activation automatique de l’application.

Les sondes enregistrent séparément : octets IQ, trames du décodeur, mots vérifiés/rejetés, insertions SQLite, lignes de chaque tableau, lignes et contenu exact des deux journaux, files restantes et état à l’arrêt. Des verrous de six secondes sont armés périodiquement et déclenchés sur une vraie trame reçue, afin de forcer la reprise sur un message réel. Les données réelles restent locales ; seuls les compteurs et résultats sont destinés au dépôt.

Une première exécution a exercé les quatre modes pendant quelques minutes, puis a été interrompue volontairement pour garder les compteurs de diagnostic exclusivement dans le fichier technique et cesser l’ouverture des anciens chemins lors de Save. Elle n’est pas comptée comme essai prolongé réussi ; `hardware-initial.log` la conserve séparément.

## Portée et limites

Poste disponible : Windows 11 Pro x64 / Intel i7-8550U, Qt 6.8.3 / MinGW 13.1. RTL OEM / tuner R828D. Pas de validation Windows 10, AMD, autres familles SDR, plusieurs récepteurs physiques simultanés ni H24. La qualité affichée représente les mots acceptés après BCH/parité, pas la proportion de messages RF retrouvés.

La conservation est limitée par les ressources physiques et les capacités de SQLite/Qt. Un disque plein ou des permissions interdisant aussi la file locale empêchent une garantie durable ; l’erreur est signalée et le message reste en mémoire tant que le processus existe. Les reçus TXT détectent une modification externe conflictuelle sans écraser le fichier ; une telle intervention peut nécessiter une résolution manuelle. Aucune preuve universelle face à panne électrique, disque défaillant ou module SDR tiers bloquant.


## Distribution

Le ZIP candidat extrait démarre nativement avec PATH limité à Windows, code 0 (`portable-test.json`). Aucun dossier Qt/MinGW du poste de développement n’est utilisé par ce démarrage. Captures natives de l’interface et des deux journaux : `qualification-017/captures/`, uniquement données fabriquées et identifiées comme tests. Le test natif vérifie séparément le suivi depuis le bas et la consultation manuelle dans les deux tableaux.

L’installateur et les empreintes des livrables finaux sont contrôlés après la fin de l’essai prolongé ; les résultats sont conservés avec les autres preuves de cette qualification. Les versions Windows/CPU et les matériels non disponibles restent non qualifiés. Vingt minutes avec les deux fichiers ne constituent pas une preuve H24.


## GitHub Actions et provenance

Le [run #23](https://github.com/krauschris95-alt/pocsag-sdr-pro/actions/runs/41989204208), commit `a600fbd0a43e9b600f7ca504f46a1506babd7e39`, est bloqué **avant démarrage** : « The job was not started because an Actions budget is preventing further use. » Aucun artefact Actions ni succès CI revendiqué. Les paquets sont compilés et contrôlés localement sur le poste Windows décrit. Le workflow actualisé demeure disponible pour une exécution lorsque le budget sera rétabli ; aucun réglage de facturation modifié.


## Contrôle final de l’installateur et du ZIP

**Réussi sur Windows 11 Pro x64** : installation silencieuse dans un dossier de test du projet, lancement natif avec PATH limité à Windows (code 0), désinstallation (code 0), application retirée et journal utilisateur ajouté dans le dossier d’installation conservé. Aucune installation utilisateur préexistante remplacée. Rapport `qualification-017/installer-test.json`, capture de démarrage isolée `installed.png`.

Le ZIP final a passé son contrôle d’intégrité ; l’exécutable qu’il contient a la même empreinte que le binaire vérifié au démarrage autonome et l’application liée à l’implémentation de l’essai prolongé. Aucun fichier de base utilisateur, préférence, capture IQ, vidéo privée ou DLL de simulation n’est inclus. Empreintes dans `qualification-017/packages-sha256.json`.

L’audit des imports ne trouve aucune dépendance requise manquante. Les plugins optionnels Qt PostgreSQL/Mimer présents ont leurs dépendances externes non fournies ; ils ne sont pas utilisés par l’application, qui emploie SQLite. Voir `dependencies.json`. Les documents embarqués ont été copiés lors de l’assemblage ; ce complément de contrôle final et ses JSON sont disponibles dans le dépôt et la Release.
