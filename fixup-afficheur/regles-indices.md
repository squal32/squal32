# Règles d'indices (validées par Dimitri le 26/10/2026)

Source : un cartouche réel (photo non conservée ici : document confidentiel).
Les exemples ci-dessous sont génériques.

## Nom de fichier
`EMETTEUR_SPECIALITE_DISCIPLINE_PHASE_TYPE_NUMERO_INDICEGED_Titre_INDICEINTERNE`

| Champ | Rôle |
|---|---|
| EMETTEUR | Émetteur |
| SPECIALITE | Spécialité (ex. G12) |
| DISCIPLINE | Discipline |
| PHASE | Phase (ex. EXE) |
| TYPE | Type de document (ex. PRE) |
| NUMERO | N° incrémental GED, 6 chiffres = n° du plan (ex. 000147) |
| INDICEGED | Indice GED, numérique : 1, 2, 3, 4… |
| Titre | Libellé du plan |
| INDICEINTERNE | Lettre : A, B, C… |

Le « Code GED » du cartouche n'a pas la spécialité (G12) alors que le nom de fichier l'a. Pas encore confirmé si c'est voulu.
Secteur, objet et niveau sont dans le cartouche mais pas dans le nom de fichier.

## Règles
1. L'**indice GED** (numérique) est l'indice officiel du plan.
2. La **lettre interne** est la version de travail. Quand l'indice GED monte (ex. 4 → 5), la lettre **repart à A**.
3. Dernier fichier d'un plan = indice GED le plus élevé, puis lettre la plus élevée.
4. **Diffusé** = case « Diffusé GED » cochée dans le tableau de suivi (à créer).
   Par défaut, un PDF présent dans le dossier = « à confirmer » (pas encore diffusé confirmé).
5. L'écran : l'utilisateur tape **147**, l'écran cherche **000147**.
6. Le n° 000147 est considéré unique sur la ligne 17. Plus tard, la clé sera affaire + numéro.

## Statuts de l'écran à refaire avec cette règle
- À JOUR : le dernier fichier est diffusé (case cochée).
- À CONFIRMER : PDF présent, case non cochée.
- PÉRIMÉ : l'indice GED du papier est inférieur à l'indice diffusé.
- INTROUVABLE : aucun fichier.

## À confirmer
- Affaire sur l'écran = la ligne (L17) ou l'ouvrage ?
- Le « G12 » absent du Code GED : voulu ?
- Autres types de documents que PRE (notes de calcul, etc.) : autres codes ?
- Colonnes exactes du tableau de suivi à créer.

## Attention
`maquette.html` (v2) utilise encore l'ancien modèle : lettres A, B, C = indices finaux, initiales = brouillon.
**C'est faux d'après les règles ci-dessus.** La maquette est à refaire avec indice GED numérique + lettre interne.
