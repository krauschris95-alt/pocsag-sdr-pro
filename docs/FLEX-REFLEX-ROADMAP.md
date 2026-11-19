# FLEX / ReFLEX — réception et détection AUTO (travail en cours)

## État réel du produit

La version publiée 0.1.19 décode **POCSAG uniquement** (512/1200/2400 bauds). Son mode AUTO actuel ne détecte pas FLEX ni ReFLEX. **Cette branche ne fournit pas encore de décodeur FLEX/ReFLEX** : elle introduit seulement une fondation testée pour arbitrer entre des protocoles *déjà décodés et validés*. Ne pas afficher FLEX ou ReFLEX comme fonctionnels, ni publier une release « FLEX/ReFLEX », avant les étapes ci-dessous.

## Fonctionnement cible

- Sur **la fréquence SDR actuellement accordée**, écouter les protocoles compatibles en parallèle ; ne pas scanner/changer la fréquence pour choisir une norme.
- Modes manuels : POCSAG, FLEX, ReFLEX ; mode **AUTO** par défaut uniquement après qualification.
- Le mode AUTO ne choisit un protocole que sur des **trames validées par son décodeur** (synchronisation, correction/contrôle d'intégrité applicables), jamais sur une crête RF, un préambule isolé ou une simple ressemblance de débit.
- Conserver POCSAG pendant que des trames POCSAG valides arrivent. Exiger plusieurs trames de la nouvelle norme et une période de silence de l'ancienne pour changer le protocole indiqué. Les messages réellement validés de plusieurs protocoles peuvent néanmoins être conservés si les réseaux coexistent.
- Lors d'un réaccord ou d'une reconnexion, réinitialiser les états de synchronisation et l'arbitrage ; conserver l'historique déjà reçu.
- Afficher clairement **AUTO — POCSAG / FLEX / ReFLEX / En recherche**, sans inventer de messages.

## Étapes d'implémentation nécessaires

1. **Architecture des trames** : interface commune pour décodeurs, horodatage et fréquence exacts, protocole distinct, métriques d'intégrité, détection fiable, diagnostics et tests déterministes. `src/protocol_selector.h` est une première fondation de sélection uniquement.
2. **FLEX** : vrai démodulateur et décodeur pour variantes 1600/3200/6400 bit/s (2FSK/4FSK selon la variante), synchronisation, contrôle d'erreur, adressage, trames, texte, messages numériques, fragmentation et tests à partir de vecteurs de référence indépendants. Examiner les implémentations libres existantes et **vérifier leurs licences et attributions** avant toute réutilisation.
3. **ReFLEX** : étude séparée des variantes et du sens descendant/montant de cette radiomessagerie bidirectionnelle ; ne jamais traiter automatiquement toute trame FLEX comme ReFLEX. Préciser la portée **réception uniquement**, et obtenir des captures IQ de référence et la documentation technique nécessaires avant d'affirmer une prise en charge.
4. **UI / données** : sélectionner AUTO / POCSAG / FLEX / ReFLEX ; indiquer la norme détectée et la confiance ; enregistrer un champ `protocol` par message via migration SQLite additive. Distinguer les espaces d'adresses pour éviter de confondre un capcode POCSAG et une adresse FLEX/ReFLEX identique. Préserver le format TXT POCSAG existant et les réglages/alias.
5. **Voix / Pager** : respecter les interrupteurs individuels. Ne pas appliquer arbitrairement la règle de lecture P0 des interventions POCSAG à des messages FLEX/ReFLEX. Conserver les textes originaux et les WAV/JSON PrivateRadio existants ; aucune modification du serveur officiel.
6. **Qualification** : bruit seul, POCSAG seul, FLEX seul, ReFLEX seul, coexistence, bascules, signaux faibles, pertes de sync, plusieurs SDR, 24 h, capture réelle. Distinguer les tests synthétiques des tests matériels et ne pas annoncer une compatibilité non démontrée.

## Tests de la fondation

`tests/protocol_selector_tests.cpp` vérifie le maintien de POCSAG, le basculement FLEX/ReFLEX après trames confirmées, l'hystérésis, les délais, les modes forcés et la remise à zéro après réaccord. Il ne prouve **aucun décodage RF FLEX/ReFLEX**.

## Conditions avant publication

Ne pas fusionner une interface qui prétend recevoir FLEX/ReFLEX tant que les décodeurs ne fonctionnent pas réellement. Pas de modification de la production PrivateRadio. La version 0.1.19 demeure la base opérationnelle POCSAG.
