# FixUp Afficheur

Petit écran tactile ESP32 intégré au FixUp (support mural de plans A0 à A6).
Il indique si le plan papier affiché est bien le dernier indice diffusé au chantier.

## Fichiers
- `regles-indices.md` : règles d'indices et nom de fichier (à lire en premier).
- `maquette.html` : maquette cliquable (ouvrir dans un navigateur, fonctionne seule).
  Données inventées. Panneau « Simulation » : âge de la synchro, pile, Wi-Fi, veille.

## Principe validé
1. L'utilisateur choisit l'affaire (ex. L17), tape le n° de plan, lit le statut.
2. L'écran lit une copie des données gardée en mémoire flash. Il n'interroge pas le serveur à chaque fois.
3. Synchro : le Wi-Fi s'allume, télécharge, se coupe. Heure de dernière synchro affichée en grand.
4. Bouton « Synchro » sur les écrans. Si pas de Wi-Fi connu : bouton « Partage de connexion » (téléphone).
5. Choix d'un réseau Wi-Fi par liste + mot de passe (clavier à l'écran). Réseaux mémorisés.
6. Écran noir au repos. Un appui réveille. Alimentation : piles AA.

## Statuts
À JOUR (vert) · NOUVEL INDICE / NON DIFFUSÉ / EN COURS (orange) · PÉRIMÉ (rouge, si l'indice du papier saisi diffère de l'indice diffusé) · INTROUVABLE (gris).
Synchro : orange au-delà de 24 h ou heure inconnue, rouge au-delà de 3 jours.

## Matériel (à confirmer)
CYD ESP32-2432S028, 2,8", 320×240, tactile résistif. Je n'ai pas pu vérifier que c'est le même
appareil que celui de la discussion « assistant de bureau » : à confirmer.

## À tester sur la carte
- Broche de réveil tactile (T_IRQ, probablement GPIO36).
- Broche du rétroéclairage (GPIO21 ou 27 selon modèle).
- Courant de veille réel, écran noir, avec et sans tactile actif.
- Décision : réveil tactile ou bouton physique.
- Taille de la flash (4 Mo annoncés, non confirmé).
- Piles retirées : la flash garde les données, l'heure est perdue (resynchroniser).

## Estimations d'autonomie (calculs, pas des mesures)
Environ 20 consultations par jour + 1 synchro = ~12 mAh/jour.
Avec 3000 mAh : veille 6,5 mA (carte telle quelle) ≈ 3 semaines ; veille 100 µA ≈ 5 mois.
La veille décide de tout. Piles conseillées : lithium AA non rechargeable (4 piles, à valider).

## Plus tard
Accès au serveur depuis l'extérieur, lecture du tableau Excel de suivi, contrôle du cartouche, bordereaux.
