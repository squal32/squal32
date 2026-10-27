# Étape 4 — interface LVGL (données de démonstration)

Reproduit la maquette validée sur la vraie carte : accueil, pointage, focus (minuteur 25/5 et 50/10),
vision (huit exercices, arrêt immédiat), trajets, compagnon animé dans la barre du bas.

**Tout ce qui vient de Home Assistant est fictif** (alarme, pas, trajets) et marqué « fictif ».
L'heure affichée est une heure de démonstration (08:42 au démarrage), pas la vraie heure.
Pas encore de Wi-Fi, de sauvegarde des pointages, ni d'alerte alarme : ce sont les étapes suivantes.

## Utilisation
1. Ouvre ce dossier (`02-interface`) dans VS Code, carte branchée.
2. Upload (→). La première compilation télécharge LVGL et TFT_eSPI.
3. **Premier démarrage : calibration du tactile.** Touche le centre de la croix, puis la 2e croix.
   Les valeurs sont enregistrées. Pour recalibrer : maintiens l'écran pendant le démarrage.
4. Essaie les quatre onglets, les gros boutons, un exercice de vision, le minuteur.

## Calibration du tactile (4 croix)
Au premier démarrage (ou en maintenant l'écran au démarrage) : touche le centre de chaque croix,
dans l'ordre haut gauche, haut droite, bas gauche, bas droite, en levant le doigt entre chaque.
Un test de 6 secondes suit : un point vert doit apparaître exactement sous ton doigt.
Le port série affiche aussi chaque appui (`appui x,y`) pour vérifier la position lue.
