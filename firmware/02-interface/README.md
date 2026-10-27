# clin — interface LVGL

**clin** (comme un clin d'œil) : le petit compagnon qui cligne des yeux et prend soin des tiens.

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
1. **Depuis l'écran** : **appui long sur le compagnon** (barre du bas) → Réglages → Wi-Fi
   (ou le nom du Wi-Fi en haut à droite de l'accueil), choisis le réseau (box ou partage de connexion du téléphone), tape le mot de passe sur le clavier.
   Trois réseaux au plus sont gardés en mémoire ; la carte passe de l'un à l'autre toute seule.
   Seuls les réseaux 2,4 GHz sont visibles.
   Facultatif : `src/secrets.h` (copie de `secrets.example.h`, jamais publié) pour préremplir des réseaux.
2. Sans Wi-Fi : touche l'heure sur l'accueil pour la régler à la main (« heure manuelle » s'affiche).
   La carte n'a pas de pile : après une coupure de courant, l'heure manuelle est perdue.

## Réglages
Appui long (environ une demi-seconde) sur le compagnon au milieu de la barre du bas :
Wi-Fi, heure manuelle, et un emplacement réservé pour le prochain projet.

## Démarrage
Écran d'accueil « clin » : le compagnon ouvre les yeux, cligne, sourit, barre de chargement (environ 3 s).

## Veille
Après 1 minute sans toucher l'écran, si aucun minuteur ni exercice ne tourne, l'écran passe en veille :
le compagnon seul, en grand, avec le nom « clin » et l'heure, qui cligne, regarde à gauche et à droite, sourit. Rétroéclairage baissé.
Au bout de 5 minutes l'écran s'éteint complètement. Un appui le rallume sans déclencher de bouton.

## Tactile
La correspondance du tactile vient du diagnostic à 9 cibles (`03-diag-tactile`) et fonctionne sans calibration.
Pour recalibrer : maintiens l'écran au démarrage, puis touche les 9 cibles (lève le doigt entre chaque).
Le port série affiche chaque appui (`appui x,y (brut …)`).
