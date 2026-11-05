# Préparation audio PrivateRadio — contrat LAB v2

## État et périmètre

POCSAG SDR Pro 0.1.11 produit réellement des WAV PCM et des manifestes JSON locaux. Aucun accès au serveur officiel, changement Caddy/Tailscale/réseau, création de canal ou modification Android. Le relais distant est OFF par défaut. L’application livrée ne construit aucun adaptateur réseau : ON signifie attente d’un adaptateur raccordé et validé, sans transmission. HttpsPagerTransport est éprouvé uniquement au laboratoire ; il ne définit pas l’API réelle de PrivateRadio 3.0.

## Sélection et indépendance

Un message ALPHA ou NUMERIC accepté d’un capcode enregistré avec Actif ON et Pager ON crée une tâche persistante dans la même transaction SQLite que le message. Voix locale, affichage inférieur et TXT restent indépendants. Refus TONE/diagnostics conservé. Les tâches déjà autorisées gardent leur identité ; les modifications de capcode concernent les nouvelles arrivées. Un capcode non enregistré ou Pager OFF ne crée aucune tâche.

Dossier et expiration capturés à l’arrivée. UUID de message dans le nom WAV ; jamais le seul capcode. Synthèse SAPI directe, mono 22 050 Hz / PCM 16 bits, sans capture des haut-parleurs ni Virtual Audio Cable. Alias, pause de 300 ms, message complet ; XML échappé.

## Mode A — même ordinateur

Dans Outils → PrivateRadio → Dossier des annonces vocales, choisir un dossier accessible aux comptes Windows explicitement autorisés. Écriture dans un sous-dossier temporaire privé du même volume, contrôle RIFF/PCM et SHA-256, renommage du WAV, publication atomique de nom.wav.json. Le consommateur doit observer uniquement les manifestes prêts, jamais les sous-dossiers .pager-work-* ni les WAV seuls.

Le manifeste est un instantané de publication locale ; SQLite reste la référence de génération et transfert. Aucun consommateur officiel ni accusé par dépôt de fichier implémenté. L’adaptateur devra rendre l’accusé au producteur sans écrire directement dans sa base SQLite.

Les fichiers contiennent les interventions privées : ACL adaptées nécessaires. Aucune URL publique ou serveur HTTP local créé. Aucun changement de droits Windows, partage ou identifiants réseau.

## Données v2

Champs v1 conservés : schema, message_id, capcode sur sept chiffres, alias, timestamp_utc, frequency_hz, source, text, utterance. v2 ajoute generation_state, transfer_state, expires_utc et audio :

| Champ audio | Valeur |
| --- | --- |
| path | Chemin local complet, exclusivement pour le même PC |
| filename | Nom unique du WAV |
| format | audio/wav; codecs=1 |
| sample_rate | 22050 |
| channels | 1 |
| bits_per_sample | 16 |
| duration_seconds | Durée calculée depuis les échantillons PCM |
| sha256 | Empreinte du fichier complet |

Le helper v1 génère toujours schema=1 ; la file audio utilise schema=2. Un consommateur strict doit négocier la version. Identité stable lors des reprises, hash associant les octets à l’événement, dates UTC ISO 8601 avec millisecondes. Expiration vide : aucune expiration.

## Mode B — adaptateur HTTPS de laboratoire

Aucune adresse, port, clé ou URL PrivateRadio réelle présumé. Le LAB utilise 127.0.0.1, un port éphémère, certificat temporaire avec SAN local et jeton synthétique ; aucun service existant touché.

L’adaptateur reçoit l’URL explicite du banc. HTTPS et Bearer exigés, vérification chaîne/nom du certificat, aucune redirection suivie ou erreur TLS ignorée. Délais bornés, aucune réponse brute ni clé journalisée.

Le POST LAB contient event et audio_base64. event est l’enveloppe v2 sans audio.path : le serveur reçoit les octets WAV, pas un chemin Windows inutilisable. Idempotency-Key égale message_id. Réponse : accepted=true et message_id identique. Refus, accusé absent/différent ou interruption : reprise avec le même identifiant.

Ce POST LAB n’est pas une route PrivateRadio. Découverte, URL, authentification réelle, format final et négociation devront être adaptés après étude de l’API dans l’espace serveur séparé.

## Persistance, reprises et expiration

pager_jobs conserve enveloppe, chemin capturé, états, tentatives, prochain essai et erreur technique. Génération : pending, retry, ready, expired. Transfert : disabled, waiting_adapter, retry, acknowledged, expired. Perte de l’accusé : renvoi du même événement ; le serveur doit acquitter à nouveau sans rediffuser.

WAV absent : régénération. Fichier existant incomplet : conservé avec erreur, jamais écrasé automatiquement. WAV valide mais publication/mise à jour interrompue : reprise sans synthèse inutile. Configuration permettant de réaffecter explicitement les tâches différées sans WAV. Aucun effacement automatique d’audio, TXT ou historique.

Expiration contrôlée avant génération et avant publication/transfert. Le serveur devra vérifier expires_utc lors de la diffusion : un appel en cours peut dépasser la limite client. Sous 16 Mio libres sur le volume audio, génération différée et tâche conservée. File vocale locale distincte, avec sa politique existante.

## Travail restant côté PrivateRadio

Étudier l’API et le canal Pager existants, définir l’authentification réelle, brancher consommateur local ou transport adapté, vérifier droits des utilisateurs, priorités SOS/PTT, codec, accusés durables, déduplication, expiration et clients. Ne déclarer la liaison opérationnelle qu’après essais bout en bout. Aucun de ces éléments officiels n’est modifié par cette version.
