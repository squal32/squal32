# Étape 2 — identification de la carte (sortie série uniquement)

Aucune donnée privée ici. Ce programme n'écrit rien à l'écran : il lit les identifiants du contrôleur,
teste le tactile, la LED RGB, le rétroéclairage et le capteur de lumière.

## Sur ton PC (Windows)
1. Installe **Visual Studio Code**, puis l'extension **PlatformIO IDE**.
2. Ouvre le dossier `firmware/00-identification`.
3. Branche l'ESP32 en USB (essaie le port USB-C, sinon le micro-USB). Si aucun port n'apparaît,
   installe le pilote **CH340** (ou CP210x selon la puce près du connecteur).
4. Clique sur la flèche **Upload** de PlatformIO. Si le téléversement échoue : maintiens le bouton **BOOT**
   pendant « Connecting… », relâche quand ça démarre.
5. Ouvre le **Serial Monitor** (115200 bauds), appuie sur **RESET** si besoin.
6. Touche les 4 coins de l'écran puis le centre.
7. Copie-colle-moi tout ce que le moniteur affiche, avec : la référence écrite au dos de la carte
   et si le rétroéclairage a clignoté / quelle couleur de LED s'est allumée.
