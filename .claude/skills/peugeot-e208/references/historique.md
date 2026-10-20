# Historique (le plus récent en haut)

Une ligne datée par session : ce qui a été fait, décidé ou laissé en suspens.

- **20/10/2026** : demande d'un préconditionnement automatique avant le départ de 6 h 30 (maison ou zone Ornella, batterie suffisante, moins de 10 °C). Brouillon et propositions dans `projet-preconditionnement.md` ; à vérifier sur l'instance (automatisation existante ? entités).
- **20/10/2026** : demande d'un suivi à part de la borne rapide de la maison pour le syndic (une ligne par recharge et un relevé par mois). Tableaux Notion créés ; borne hors ligne, à remettre en ligne.
- **20/10/2026** : demande d'un historique complet (jour, semaine, mois, année) des km, du temps et des coûts de recharge. Page Notion « Voiture e-208 » et base « Historique quotidien e-208 » créées ; partie HA à faire en session locale (`projet-historique.md`).
- **20/10/2026** : 2 h de conduite d'affilée non affichées en un seul bloc. Règles de découpage relevées dans le code de l'add-on (`decoupage-trajets.md`). Données réelles non vérifiées : HA injoignable depuis la session cloud. Création de cette skill.
- **14/10/2026** : audit Notion, points 39, 40 et 43 concernant la voiture (voir `entites-et-fichiers.md`).
- **12/09/2026** : batterie, autonomie et charge « indisponibles » depuis ~10 h. Cause : HA ne résout pas `api.groupe-psa.com` (« Name or service not known ») alors que le reste du réseau le résout. Redémarrer l'add-on et le plugin DNS n'a pas suffi. Proposé : forcer les DNS 1.1.1.1 / 8.8.8.8 avec `ha dns options`. **Résultat final non documenté.** Le même jour : connexion du MCP Home Assistant de Claude Code (Windows) ; jeton stocké hors dépôt.
- **03/09/2026** : l'onglet Trips était vide ; cause : cache de la page (Ctrl+Maj+R). Ni le jeton ni le calcul n'étaient en cause, 1027 trajets étaient en base. Le redémarrage auto toutes les 48 h a été écarté. Le même jour : création de `sensor.psa_dernier_trajet`, de la page « Ma voiture », de la carte des coûts de recharge et du graphique du temps de conduite avec popup par jour.
