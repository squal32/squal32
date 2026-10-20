# Découpage des trajets dans PSA Car Controller

Source : `psa_car_controller/psacc/repository/trips.py` et `psacc/application/trip_parser.py`, identiques en v3.7.9 et sur `master` au 20/10/2026.

L'add-on ne reçoit pas les trajets de Peugeot : il les reconstruit à partir des positions enregistrées (horodatage, kilométrage, batterie). Entre deux positions consécutives :

1. **Arrêt ou lenteur** : vitesse moyenne < 0,2 km/h **et** écart > 0,05 h (3 min) → le trajet redémarre. Un bouchon, une pause, ou un kilométrage que l'API renvoie figé pendant plus de 3 min coupent donc le trajet.
2. **Recharge détectée** : batterie qui monte de plus de 2 % sans distance, ou de plus de 5 % avec distance → coupure.
3. **Trou de plus de 2 h** entre deux positions → fin du trajet.
4. **Trajet rejeté sans message visible** (log DEBUGV « trip discarded ») si :
   - conso > `max_elec_consumption` (70 kWh/100 km par défaut),
   - ou vitesse moyenne ≥ 150 km/h.
   Son temps disparaît des totaux.
5. Un trajet de distance 0 n'est pas gardé.
6. La durée va de la **première à la dernière position reçue** : le temps avant le premier point et après le dernier n'est pas compté.

## Côté page « Ma voiture »
- La carte « Dernier trajet » n'affiche que le **dernier morceau**.
- `sensor.psa_dernier_trajet` se met à jour toutes les **15 min**.
- Le graphique additionne les trajets par jour : un trajet à cheval sur minuit se répartit sur deux jours.

## Vérifier un trajet
1. Cliquer sur la barre du jour dans le graphique : plusieurs trajets = découpage ; total bien inférieur au temps réel = morceaux rejetés ou points manquants.
2. Pour la preuve : ajouter `-d 9` à `PSACC_OPTIONS`, redémarrer l'add-on, lire le log (`restart trip`, `stop trip`, `too much time detected`, `trip discarded`, `Trip: …`), puis remettre `-r --web-conf -c`.
