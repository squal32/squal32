# Étape 3 — test d'affichage

Deux variantes de contrôleur sont possibles sur cette carte (ILI9341 ou ST7789), et les sources
publiques ne sont pas d'accord entre elles. On teste donc les deux.

1. Ouvre ce dossier dans VS Code.
2. Barre du bas : clique sur « Default (...) » et choisis **env:ili9341**. Upload (→).
3. Regarde l'écran : barres ROUGE, VERT, BLEU, BLANC, NOIR, cadre jaune, repères 1 à 4 dans les coins.
4. Touche l'écran : les couleurs s'inversent, l'état s'écrit en haut.
5. Si l'image est vide, déformée ou fausse dans les deux états, relance avec **env:st7789**.
6. Dis-moi : variante utilisée, si les couleurs sont correctes (et avec quel état d'inversion),
   si le texte est lisible et dans le bon sens, et où se trouvent les repères 1 à 4.
