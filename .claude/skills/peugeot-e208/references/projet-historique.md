# Projet : historique complet (km, temps, recharges) — demandé le 20/10/2026

## Objectif
Garder pour toujours, par jour, semaine, mois et année : km, temps de conduite, nombre de trajets, kWh et coûts de recharge (Type F lente, Type 2 rapide, extérieur Chargemap).
1. **Notion** : une ligne par jour, envoyée par HA.
2. **Page « Ma voiture »** : dans la carte Kilométrage, les boutons Jour / Semaine / Mois / Année ouvrent un popup avec l'historique de toutes les périodes passées.

## Déjà fait (20/10/2026, depuis Notion)
- Page Notion « Voiture e-208 » sous « Projet HA AquarelleHom ».
- Base « Historique quotidien e-208 » : database_id `59e67838d8504eca938beffe15d4a3fd`, data source `f84d42b0-f8a4-41ae-b26e-3a0326959b8c`.
- Colonnes à remplir : `Jour` (titre, AAAA-MM-JJ), `Date`, `Km`, `Temps de conduite (min)`, `Trajets`, `Compteur km`, `kWh Type F`, `Coût Type F`, `kWh Type 2`, `Coût Type 2`, `kWh extérieur`, `Coût extérieur`, `Source` (Auto HA / Rattrapage / Manuel).
- Colonnes calculées par Notion : `Coût total`, `kWh total`, `Semaine`, `Mois`, `Année`.
- Vues : Par semaine, Par mois, Par année (le regroupement sur la colonne formule n'a pas pris via l'API : à régler dans Notion avec Grouper → Semaine / Mois / Année), graphiques « Km par mois » et « Coût recharge par mois ».

## À faire côté Home Assistant (session locale, avec accès HA)
Vérifier chaque entité sur l'instance avant d'écrire ; ne rien inventer.

1. **Jeton Notion** : Dimitri crée une intégration interne Notion et la connecte à la page « Voiture e-208 ». Le jeton va dans `secrets.yaml` (`notion_token`), jamais dans la conversation ni dans ce dépôt.
2. **Compteurs du jour** :
   - Type F et Type 2 : `utility_meter` quotidiens sur les capteurs d'énergie existants (Type 2 = `sensor.compteur_borne_de_charge_rapide_energy_monthly_02516` ou sa source ; Type F = à identifier). Coût = kWh × 0,2146 €/kWh (vérifier le tarif actuel).
   - Extérieur : le popup « + » de la page alimente aussi un compteur du jour (kWh et €), remis à zéro après l'envoi.
3. **Km, temps et trajets du jour** : étendre le script de `sensor.psa_dernier_trajet` (déjà regroupé par jour) pour exposer aussi km et nombre de trajets par jour. `Compteur km` = kilométrage de `sensor.psa` en fin de journée.
4. **Envoi quotidien** : automatisation à 23:55 → `rest_command` `POST https://api.notion.com/v1/pages` (en-têtes `Authorization: Bearer` depuis `secrets.yaml`, `Notion-Version: 2022-06-28`, parent `database_id` ci-dessus), `Source` = « Auto HA ». Notifier (Telegram) si l'envoi échoue.
5. **Rattrapage du passé** (une fois) :
   - km, temps, trajets : recalculables jour par jour depuis tous les trajets de l'add-on (`/vehicles/trips`) → lignes « Rattrapage ».
   - kWh et coûts : seulement si les statistiques long terme de HA les ont gardés (capteurs avec `state_class`). Les recharges extérieures passées ne sont pas récupérables, sauf le mois en cours. Dire clairement ce qui manque.
6. **Page `voiture.html`** : boutons Jour / Semaine / Mois / Année de la carte Kilométrage → popup historique (km, temps, kWh, coûts par période, plus récent en haut).
   - Source des données : Home Assistant (attributs d'un capteur d'historique exclu du recorder, et/ou statistiques long terme). Ne jamais appeler Notion depuis la page : le jeton serait visible dans le navigateur.
   - Regarder d'abord comment la page lit déjà les états HA, puis réutiliser la même méthode.
   - Changer le paramètre anti-cache de l'iframe après modification.
7. Tester : une ligne réelle arrive dans Notion, les popups s'ouvrent pour les 4 périodes, puis mettre à jour `historique.md` et `entites-et-fichiers.md`.

## Borne rapide de la maison (Type 2, 7,3 kW) — suivi pour le syndic (demandé le 20/10/2026)
La conso de la borne rapide doit être envoyée régulièrement au syndic : elle a ses propres tableaux, à part de l'historique quotidien.
La borne est hors ligne au 20/10/2026 ; Dimitri va la remettre en ligne.

Déjà créé dans Notion (sous « Voiture e-208 ») :
- **Recharges borne rapide (maison)** : database_id `f609102385a241dd995d352ed8de3cc2`. Une ligne par recharge : `Recharge` (titre AAAA-MM-JJ HH:MM), `Début`, `Fin`, `Durée (min)`, `kWh`, `Index début (kWh)`, `Index fin (kWh)`, `Coût`, `Tarif (€/kWh)`, `Source` ; `Mois` est calculé. Graphique « kWh par mois ».
- **Relevés mensuels borne rapide (syndic)** : database_id `57f217d9763a4863b1c0c038d2570d6e`. Une ligne par mois : `Mois` (titre AAAA-MM), `Période`, `Index début (kWh)`, `Index fin (kWh)`, `kWh`, `Nombre de recharges`, `Coût`, `Tarif (€/kWh)`, `Données complètes`, `Envoyé au syndic`, `Envoyé le`, `Remarque`. Vue « À envoyer au syndic ».

À faire côté HA :
1. Remettre la borne en ligne, identifier ses entités réelles (compteur d'énergie total en kWh, puissance, état). Le relevé du syndic se base sur l'**index du compteur** (début et fin), pas sur un compteur remis à zéro.
2. **Surveillance** : alerte Telegram si la borne est `unavailable` plus de 15 min, et quand elle revient ; alerte si une charge démarre alors que la voiture n'est pas branchée/à la maison (à discuter).
3. **Une ligne par recharge** : début quand la puissance dépasse un seuil (ex. > 500 W) pendant 1 min, fin quand elle retombe sous le seuil pendant 5 min ; envoyer index début/fin, kWh, durée, coût.
4. **Relevé mensuel** : le 1er du mois à 00:05, créer la ligne du mois écoulé (index au 1er à 00:00 du mois et du mois suivant, kWh, nombre de recharges, coût). Cocher `Données complètes` seulement si la borne n'a pas été hors ligne pendant le mois.
5. Rattrapage : uniquement à partir des statistiques long terme de HA, si elles existent ; sinon, partir d'un relevé manuel de l'index à la remise en ligne.
6. Lien Notion (audit du 14/10/2026), point 40 : « Facturation électricité Grégory + conso borne rapide fin de mois : capteur/helper manquants ». À traiter en même temps.
