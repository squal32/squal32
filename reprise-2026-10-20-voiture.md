# Reprise — Voiture e-208 et Home Assistant (session cloud du 20/10/2026)

À lire en début de session **sur le Mac** (accès HA via `~/.config/homeassistant/credentials.sh`). Réponses brèves, en français. Ne rien affirmer sans l'avoir vérifié sur l'instance.

## 1. Installer la skill en local (à faire en premier)
Toute la mémoire sur la voiture est dans la skill `peugeot-e208` du dépôt public GitHub `squal32/squal32`, branche `claude/kind-cori-7ir37v`, dossier `.claude/skills/peugeot-e208/`.

```bash
mkdir -p ~/.claude/skills && cd /tmp && rm -rf squal32-reprise \
  && git clone -q --depth 1 -b claude/kind-cori-7ir37v https://github.com/squal32/squal32.git squal32-reprise \
  && cp -R squal32-reprise/.claude/skills/peugeot-e208 ~/.claude/skills/ \
  && ls ~/.claude/skills/peugeot-e208
```

Puis lire `~/.claude/skills/peugeot-e208/SKILL.md` et les fichiers de `references/`.

## 2. Ce qui a été fait le 20/10/2026 (session cloud, sans accès HA)
- **Conversation retrouvée** : session « Remontée d'information des trajets » du 03/09/2026 (onglet Trips vide = cache du navigateur ; création de `sensor.psa_dernier_trajet`, de la page « Ma voiture », des coûts de recharge, du graphique du temps de conduite).
- **2 h de conduite non affichées d'un bloc** : l'add-on PSA Car Controller découpe les trajets (arrêt > 3 min, kilométrage figé, recharge, trou > 2 h, trajets rejetés). Détail dans `references/decoupage-trajets.md`. **Pas encore vérifié sur les vraies données.**
- **Skill `peugeot-e208` créée** (voir § 1).
- **Notion** :
  - Page « Skills Claude — Domotique Aquarelle » (index des skills HA) sous « Projet HA AquarelleHom ».
  - Page « Voiture e-208 » sous « Projet HA AquarelleHom », avec 3 tableaux :
    - « Historique quotidien e-208 » (une ligne par jour : km, temps, trajets, kWh et coûts Type F / Type 2 / extérieur) ;
    - « Recharges borne rapide (maison) » (une ligne par recharge) ;
    - « Relevés mensuels borne rapide (syndic) » (un relevé par mois, case « Envoyé au syndic »).
  - Les vues Par semaine / Par mois / Par année n'ont pas pris le regroupement : Dimitri doit faire Grouper → Semaine / Mois / Année dans Notion.

## 3. À faire maintenant, dans l'ordre (tout est détaillé dans la skill)
1. **Vérifier le trajet de 2 h du 20/10/2026** : popup du jour sur la page « Ma voiture », onglet Trips de l'add-on ; si besoin, logs avec `-d 9` (puis remettre `-r --web-conf -c`).
2. **Préconditionnement du matin** (`references/projet-preconditionnement.md`) :
   - chercher d'abord une automatisation de préconditionnement existante et l'améliorer ;
   - départ 6 h 30 au plus tard, voiture à la maison ou dans la zone « Ornella », moins de 10 °C, batterie suffisante ;
   - propositions à faire valider : 30 min d'avance (+15 min s'il gèle), 40 % mini hors charge, refroidissement au-dessus de 27 °C, interrupteur on/off, jours fériés exclus, message Telegram à chaque décision ;
   - remplacer toutes les entités `# À VÉRIFIER` du brouillon, tester sans attendre le matin.
3. **Historique quotidien → Notion** et **popups Jour / Semaine / Mois / Année** de la carte Kilométrage (`references/projet-historique.md`, partie 1). Une session a peut-être déjà été lancée pour ça depuis l'app : vérifier où elle en est avant de refaire.
4. **Borne rapide pour le syndic** (`references/projet-historique.md`, partie « Borne rapide ») : une fois la borne remise en ligne, surveillance + une ligne par recharge + relevé mensuel basé sur l'index du compteur ; traiter aussi le point 40 de l'audit Notion.
5. À la fin : ajouter une ligne datée dans `references/historique.md` de la skill, et pousser les changements sur la branche `claude/kind-cori-7ir37v` (dépôt **public** : ni URL, ni jeton, ni IP, ni VIN).

## 4. Règles
- Jeton Notion pour HA : intégration interne connectée à la page « Voiture e-208 », stockée dans `secrets.yaml`. Dimitri le colle lui-même, jamais dans la conversation.
- Ne jamais afficher la zone de `configuration.yaml` juste avant `rest:` (elle contient une clé).
- Valider la config avant chaque rechargement ; demander avant un redémarrage complet de HA.
