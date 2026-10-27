# Étape 4 — interface LVGL (données de démonstration)

Reproduit la maquette validée sur la vraie carte : accueil, pointage, focus (minuteur 25/5 et 50/10),
vision (huit exercices, arrêt immédiat), trajets, compagnon animé dans la barre du bas.

**Tout ce qui vient de Home Assistant est fictif** (alarme, pas, trajets) et marqué « fictif ».
Pas encore de Wi-Fi, de sauvegarde des pointages, ni d'alerte alarme : ce sont les étapes suivantes.

## Utilisation
1. Ouvre ce dossier (`02-interface`) dans VS Code, carte branchée.
2. Upload (→). La première compilation télécharge LVGL et TFT_eSPI.
3. **Premier démarrage : calibration du tactile.** Touche le centre de la croix, puis la 2e croix.
   Les valeurs sont enregistrées. Pour recalibrer : maintiens l'écran pendant le démarrage.
4. Essaie les quatre onglets, les gros boutons, un exercice de vision, le minuteur.

## Heure réelle
L'heure vient d'Internet (NTP) dès que la carte est connectée au Wi-Fi, heure de Paris.
1. Copie `src/secrets.example.h` en `src/secrets.h` et mets le nom et le mot de passe de ton Wi-Fi
   (deux réseaux possibles : maison et partage du téléphone). Ce fichier n'est jamais publié.
2. Sans Wi-Fi : touche l'heure sur l'accueil pour la régler à la main (« heure manuelle » s'affiche).
   La carte n'a pas de pile : après une coupure de courant, l'heure manuelle est perdue.

## Veille
Après 1 minute sans toucher l'écran, si aucun minuteur ni exercice ne tourne, l'écran passe en veille :
le compagnon seul, en grand, qui cligne, regarde à gauche et à droite, sourit. Rétroéclairage baissé.
Au bout de 5 minutes l'écran s'éteint complètement. Un appui le rallume sans déclencher de bouton.

## Tactile
La correspondance du tactile vient du diagnostic à 9 cibles (`03-diag-tactile`) et fonctionne sans calibration.
Pour recalibrer : maintiens l'écran au démarrage, puis touche les 9 cibles (lève le doigt entre chaque).
Le port série affiche chaque appui (`appui x,y (brut …)`).
