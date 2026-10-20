---
name: peugeot-e208
description: Contexte complet de la Peugeot e-208 (voiture électrique) de Dimitri dans Home Assistant — add-on PSA Car Controller, trajets, compteur de temps de conduite, page « Ma voiture », coûts de recharge (Type F, Type 2, Chargemap), commandes à distance, pannes déjà diagnostiquées. Utiliser dès que la conversation parle de la voiture, de la Peugeot / e-208, de PSA Car Controller (psacc), des trajets, du kilométrage, de la batterie, de la recharge, de la borne ou de la page voiture du tableau de bord, même sans citer la skill.
---

# Peugeot e-208 — contexte Home Assistant

Réponses brèves, en français. Vérifier sur l'instance avant d'affirmer : ce fichier décrit l'état connu à une date, pas l'état actuel.

## Le véhicule
- Peugeot **e-208** 100 % électrique, application officielle **MyPeugeot**.
- Le service « My Trip Report » de MyPeugeot n'est **pas compatible** avec la e-208 (vérifié le 03/09/2026) : ne pas le reproposer.

## Architecture
- Add-on HA **PSA Car Controller** (projet `flobz/psa_car_controller`), slug `b9f12d83_psacc`, version 3.7.9 au 12/09/2026.
- Options de l'add-on : `PSACC_OPTIONS = -r --web-conf -c`
  - `-r` enregistre les positions : **sans lui, plus aucun trajet n'est capté**.
  - `--web-conf` évite un plantage si un fichier de config manque ; `-c` garde le contrôle de charge.
  - `-d 9` = logs DEBUGV pour diagnostiquer les trajets ; à retirer après.
- API interne de l'add-on, joignable depuis HA Core : `http://127.0.0.1:5000/` (ex. `GET /vehicles/trips` = tous les trajets en JSON, le dernier élément est le dernier trajet). Les `rest_command.psa_*` existants l'utilisent.
- Les trajets sont **recalculés** par l'add-on à partir des positions stockées (SQLite), à chaque nouvelle position et à chaque démarrage. Règles de découpage : voir `references/decoupage-trajets.md`.

Entités, fichiers et pages : `references/entites-et-fichiers.md`.
Historique des sessions et décisions : `references/historique.md`.

## Accès
- URL et jeton HA : jamais dans ce dépôt (public). Côté Mac, `~/.config/homeassistant/credentials.sh` (`HA_URL_REMOTE`, `HA_TOKEN`). En session cloud, demander à Dimitri d'autoriser le domaine HA dans le réseau de l'environnement et de fournir un jeton.
- Le MCP Home Assistant ne voit que les entités exposées à Assist : pas d'accès aux add-ons ni au shell.
- Une zone de `configuration.yaml` (juste avant `rest:`) contient une clé : ne jamais l'afficher, ajouter du contenu sans la lire.
- Jamais de jeton dans une commande, une capture ou la conversation. Méthode validée : fichier texte vide → Dimitri colle → lecture par script → suppression du fichier.

## Diagnostic rapide
| Symptôme | Cause connue | Action |
|---|---|---|
| Onglet Trips vide | Cache de la page Dash | Ctrl+Maj+R sur l'onglet Trips |
| Trajet long découpé ou temps manquant | Règles de découpage de l'add-on | `references/decoupage-trajets.md` |
| Batterie / autonomie « indisponibles », erreur « Name or service not known » | HA n'arrive pas à résoudre `api.groupe-psa.com` (DNS) | `ha dns info`, puis `ha dns options --servers dns://1.1.1.1 --servers dns://8.8.8.8`, `ha dns restart`, redémarrer l'add-on |
| Erreur 500 isolée dans le log | Serveur Stellantis | Rien à faire si isolée |
| Nouvelles données absentes | `-r` retiré des options | Remettre `-r --web-conf -c` |

Un redémarrage automatique de l'add-on toutes les 48 h a été écarté : la connexion API n'était pas en cause.

## Mettre à jour cette skill
À la fin de chaque session sur la voiture, ajouter une ligne datée dans `references/historique.md` et corriger les fichiers si une entité, un fichier ou un réglage a changé.
