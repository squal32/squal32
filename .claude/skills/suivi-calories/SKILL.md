---
name: suivi-calories
description: Suivi des calories et du déficit calorique de Dimitri pour perdre du poids (objectif Hyrox) — base Notion « Suivi calories », estimation des repas, plafond quotidien, bonus sport issu de la montre Huawei, rappel du soir. Utiliser dès que Dimitri parle de ce qu'il a mangé, de calories, de déficit, de régime, de poids, de sport du jour, de la montre Huawei (anneau Bouger, pas, distance) ou de « ce soir 21 h », même sans citer la skill.
---

# Suivi calories — contexte

Réponses brèves, en français. Ne jamais valider sans vérifier, ne pas inventer une valeur manquante, donner un niveau de confiance (élevé / moyen / faible) à la fin des réponses fondées sur une estimation ou une recherche. Si Dimitri se trompe, le dire clairement.

Ne jamais écrire dans ce dépôt (public) des poids, mensurations, taux de masse grasse ni le détail des repas : ces données restent dans Notion.

## Rôle
Dimitri annonce ce qu'il mange. Pour chaque repas : estimer les kcal avec une fourchette basse / haute, saisir une ligne dans Notion, donner le total du jour, le reste disponible, et le dire franchement quand il approche du plafond. Les écarts d'un jour se compensent par du sport et les jours suivants.

## Où sont les données (Notion, espace de Dimitri)
- Base **🍽️ Suivi calories**, dans la page SECOND CERVEAU, à côté de « MON POIDS CIBLE ». Source de données : `collection://2b6c1181-b0f6-4e6c-aeec-46b996644406`.
- Colonnes : `Repas` (titre), `Date`, `Moment` (Petit-déjeuner / Déjeuner / Collation / Dîner / Boisson / extra), `Aliments`, `kcal estimées`, `Fourchette basse`, `Fourchette haute`, `Protéines (g)`, `Sport du jour (kcal)`, `Notes`.
- Poids, mensurations, métabolisme basal : page « MON POIDS CIBLE » (objectif en cours, journal des pesées, journal des mensurations).
- Plan d'entraînement et objectif : page « Programme Hyrox – Sport Reprise » (muscu lun/mer/ven, cardio modéré mar/jeu/sam, repos et pesée le dimanche matin à jeun ; rythme visé environ 0,5 kg par semaine ; déficit modéré, protéines à chaque repas).
- Il n'existe pas de vue « total par jour » créée par Claude : Dimitri doit grouper le tableau par Date et sommer les kcal. Calculer le total à la main dans les réponses.
- Protéines : ne pas remplir sans estimation fiable.

## Règles de calcul (provisoires, à ajuster après 2 semaines de pesées)
1. **Plafond de base : 2 300 kcal par jour.** Choisi sans connaître la dépense réelle (taille, âge, activité absents) ; à recalibrer avec la perte de poids observée.
2. **Bonus sport** : 50 % des **calories actives** de la montre, plafonné à **400 kcal** par jour. Plafond du jour = 2 300 + bonus. Ne pas compter le métabolisme de repos (déjà dans le plafond).
3. **Les pas ne comptent pas** : le plafond suppose une activité quotidienne normale. Une journée exceptionnelle (longue randonnée) se traite à part.
4. Écrire dans `Sport du jour (kcal)` le chiffre brut de la montre, et le plafond ajusté dans `Notes`.
5. Dîner du soir : si le reste est inférieur à environ 600 kcal, conseiller protéines maigres et légumes, sans féculents ni boissons sucrées.

## Montre Huawei
- Modèle : Huawei Watch GT (Dimitri pense GT 4, non confirmé). Appli : Huawei Santé.
- Écran des anneaux : **Bouger** (kcal, avec objectif quotidien affiché derrière le « / »), **M'entraîner** (minutes), **Me lever** (heures).
- Écran « Distance » de l'appli : distance du jour et répartition course / marche / élévation / autre. Il ne donne pas les pas.
- **Non vérifié** : que « Bouger » corresponde bien aux calories actives et exclue le métabolisme de repos. Les études de presse citées en séance (Stanford : 27 à 93 % d'erreur ; Queensland : 9 à 43 % ; FIU : surestimation plus forte avec plus de masse grasse) indiquent que les montres surestiment souvent. D'où le coefficient de 50 %.
- À demander à Dimitri : une capture de l'écran d'accueil de l'appli, pour confirmer si le total inclut le repos.

## Rappel du soir
Dimitri veut qu'on lui demande chaque soir vers **21 h** : calories de l'anneau « Bouger », sport fait, et ce qu'il a mangé depuis le dernier repas saisi. Claude ne peut pas écrire de lui-même dans une conversation : il faut une tâche programmée (notification). **Pas encore créée** au 26/10/2026 : attendait la confirmation de l'heure (21 h) et du fuseau (Europe/Paris, supposé).

## Historique des décisions
- 26/10/2026 : base Notion créée ; premier repas saisi (déjeuner au restaurant, environ 1 800 kcal, fourchette 1 500 à 2 100) ; règles ci-dessus fixées ; première capture de la montre reçue (anneaux + écran Distance).

## Références
- `references/estimations.md` : valeurs utilisées pour estimer, pour rester cohérent d'un repas à l'autre.
