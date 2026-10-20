# Entités, fichiers et pages (état au 03/09/2026, sauf mention)

À revérifier sur l'instance avant de s'appuyer dessus.

## Capteurs et entités
- `sensor.psa` : position, batterie, kilométrage (alimenté par l'add-on).
- `sensor.psa_dernier_trajet` : créé le 03/09/2026. Intégration `command_line` (script Python, sans `curl`/`jq`), toutes les 15 min, qui appelle `http://127.0.0.1:5000/vehicles/trips`. État et attributs : dernier trajet (distance, durée, date/heure, vitesse moyenne, conso), temps de conduite par jour du mois en cours, liste des trajets du mois.
- `rest_command.psa_*` (ex. `psa_lock_doors`) : commandes à distance.
- `automation.alerte_psa_car_controller_deconnecte` : alerte de déconnexion de l'add-on (jamais déclenchée au 03/09/2026).
- `sensor.compteur_borne_de_charge_rapide_energy_monthly_02516` : énergie mensuelle de la borne Type 2.
- Coût Type 2 : un `input_number` et une automatisation, sur le même modèle que la Type F. Nom exact de l'entité non relevé.
- Recharges extérieures (Chargemap) : helpers kWh et prix, cumulés et remis à zéro chaque mois par automatisation.

## Recharge
- Prise **Type F** : 1,8 kW max. Conso et coût du mois existaient déjà avant le 03/09/2026.
- Prise **Type 2** : 7,3 kW max. Coût = kWh × **0,2146 €/kWh** (même tarif que la Type F).
- **Extérieur** : carte Chargemap, sans API. Saisie manuelle (kWh + prix) avec le bouton « + » de la page.

## Tableau de bord
- Vue `aquarelle-home/voiture` : page « Ma voiture », en iframe vers `voiture.html` (servi depuis `/local/`, donc `/config/www/`). Contenu : photo de la e-208, jauge batterie et autonomie, kilométrage jour/semaine/mois/année, carte « Dernier trajet » avec graphique du temps de conduite par jour (clic sur une barre = popup des trajets du jour avec total durée et km), total du mois « Total · 8h02 » sans les km (déjà dans « Ce mois »), carte « Coûts de recharge », 8 commandes à distance, données véhicule.
- Vue `aquarelle-home/exterieur_jardin` : carte « Ma voiture » ajoutée dans `piece.html`, qui renvoie vers la page voiture.
- Premier essai abandonné : `aquarelle-home-panel/Borne-de-charge`.
- `/local/` est fortement mis en cache : après modification, changer le paramètre anti-cache (cache-buster) de l'URL de l'iframe.

## À faire (Notion, audit du 14/10/2026)
Page « Automatisations cassées — décisions à prendre » (Projet HA AquarelleHom) :
- **43** « Voiture: Suivi e208 » utilise `device_tracker.see`, supprimé en HA 2027.5 → à réécrire.
- **39** « Calcul énergie chargée hors domicile » : helpers supprimés → désactiver ?
- **40** Conso borne rapide de fin de mois : capteur ou helper manquant.
