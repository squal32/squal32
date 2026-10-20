# Projet : préconditionnement automatique du matin (demandé le 20/10/2026)

## Besoin
- Départ au travail à **6 h 30 au plus tard**, les jours travaillés.
- La voiture est **à la maison** ou **chez Ornella** (zone HA `Ornella`, déjà créée).
- Lancer le préconditionnement avant le départ seulement si la **batterie le permet** et s'il fait **moins de 10 °C**.
- Vérifier d'abord s'il existe déjà une automatisation de préconditionnement pour la maison : l'améliorer plutôt qu'en créer une deuxième.

## Règles proposées (à valider par Dimitri)
1. Démarrage à 6 h 30 moins une avance, réglable dans `input_datetime.depart_travail` (6:30) et `input_number.precond_avance_min` (30 min).
2. **Givre** : si la température est inférieure à 3 °C, partir 15 min plus tôt.
3. Batterie : **branchée** → toujours ; **non branchée** → seulement si la batterie dépasse un seuil (`input_number.precond_batterie_min`, 40 %).
4. Été (proposition) : lancer aussi au-dessus de 27 °C, pour refroidir l'habitacle.
5. Interrupteur `input_boolean.precond_auto` pour couper le tout (vacances).
6. Jours travaillés : capteur `workday` (week-ends et jours fériés exclus), sinon lundi à vendredi.
7. Toujours prévenir sur Telegram : « lancé » (avec batterie et température), ou « pas lancé » avec la raison (batterie trop basse, voiture hors zone, etc.).

## Brouillon YAML — entités à vérifier sur l'instance
Les entités marquées `# À VÉRIFIER` sont des suppositions : les remplacer par les vraies avant de charger le fichier.

```yaml
alias: Voiture — Préconditionnement du matin (maison / Ornella)
description: Lance le préconditionnement avant le départ de 6 h 30 si froid, batterie suffisante et voiture à la maison ou chez Ornella.
mode: single
triggers:
  - trigger: time_pattern
    minutes: "/5"
conditions:
  - condition: state
    entity_id: input_boolean.precond_auto
    state: "on"
  - condition: state
    entity_id: binary_sensor.workday_sensor          # À VÉRIFIER (intégration Workday)
    state: "on"
  # Vérification de l'heure de lancement (avance + 15 min en cas de givre), faite toutes les 5 min
  - condition: template
    value_template: >
      {% set temp = states('sensor.psa_temperature_exterieure') | float(99) %}   {# À VÉRIFIER #}
      {% set avance = states('input_number.precond_avance_min') | int(30) + (15 if temp < 3 else 0) %}
      {% set depart = today_at(states('input_datetime.depart_travail')) %}
      {% set lancement = depart - timedelta(minutes=avance) %}
      {{ lancement <= now() < lancement + timedelta(minutes=5) }}
  # La voiture est à la maison ou chez Ornella
  - condition: or
    conditions:
      - condition: zone
        entity_id: device_tracker.e208               # À VÉRIFIER (tracker de la voiture)
        zone: zone.home
      - condition: zone
        entity_id: device_tracker.e208               # À VÉRIFIER
        zone: zone.ornella                           # À VÉRIFIER (nom exact de la zone)
actions:
  - variables:
      temp: "{{ states('sensor.psa_temperature_exterieure') | float(99) }}"      # À VÉRIFIER
      batterie: "{{ states('sensor.psa_battery_level') | float(0) }}"            # À VÉRIFIER
      branchee: "{{ is_state('binary_sensor.psa_charging_plugged', 'on') }}"     # À VÉRIFIER
      seuil: "{{ states('input_number.precond_batterie_min') | float(40) }}"
  - choose:
      - conditions: "{{ not (temp < 10 or temp > 27) }}"
        sequence:
          - action: telegram_bot.send_message
            data:
              message: "🚗 Préconditionnement non lancé : {{ temp }} °C, pas nécessaire."
      - conditions: "{{ not branchee and batterie < seuil }}"
        sequence:
          - action: telegram_bot.send_message
            data:
              message: "🚗 Préconditionnement non lancé : batterie {{ batterie }} % < {{ seuil }} % et voiture non branchée."
    default:
      - action: rest_command.psa_preconditioning_on  # À VÉRIFIER (commande existante de la page voiture)
      - action: telegram_bot.send_message
        data:
          message: "🚗 Préconditionnement lancé : {{ temp }} °C, batterie {{ batterie }} %{{ ' (branchée)' if branchee else '' }}. Départ prévu {{ states('input_datetime.depart_travail')[:5] }}."
```

Notes :
- `telegram_bot.send_message` : reprendre exactement l'appel utilisé par les autres alertes Telegram de la maison (cible, compte).
- Le déclencheur toutes les 5 min avec la condition sur l'heure permet de changer l'heure de départ et l'avance sans modifier l'automatisation.
- La commande de l'add-on est `GET /preconditioning/<vin>/1` (relevé dans `psa_car_controller/web/view/api.py`) ; la page voiture a déjà une commande de préconditionnement, la réutiliser.
- L'automatisation « Voiture: Suivi e208 » utilise `device_tracker.see`, supprimé en HA 2027.5 (point 43 de l'audit Notion) : si elle fournit le tracker de la voiture, la réécrire en même temps.
- Durée réelle du préconditionnement de la e-208 et température utilisée (voiture ou météo) : à vérifier, puis ajuster l'avance.
