# clin — assistant de bureau ESP32 : contexte complet

Document de reprise pour toute IA ou personne qui travaille sur le projet.
À lire avant de proposer du code, un boîtier ou une intégration Home Assistant.
Dernière mise à jour : 27 octobre 2026 (firmware v0.5).

## 1. Le projet en bref

**clin** (comme un « clin d'œil ») est un petit compagnon de bureau tactile pour Dimitri, qui travaille
en bureau d'études. Un écran 2,8 pouces affiche un petit personnage animé (deux yeux dans un visage
arrondi), l'heure, un pointage d'arrivée et de départ, un minuteur de concentration et des exercices
pour reposer les yeux. Plus tard : temps de trajet et informations venant de Home Assistant.

Usage réel :
- Au bureau, la carte est **seulement alimentée** par USB (pas d'ordinateur pour la programmer).
  Internet vient du **partage de connexion Wi-Fi du Pixel 8 Pro** ; le réseau se choisit sur l'écran.
- À la maison, programmation depuis un PC Windows (VS Code + PlatformIO, port COM5).
- Elle doit rester utile sans réseau : heure (réglage manuel possible), pointage, focus, exercices.

## 2. Matériel vérifié sur la carte réelle

Carte **ESP32-2432S028R** (« Cheap Yellow Display », circuit jaune, marque Guition),
version à deux ports (USB-C + micro-USB). Vérifié par les programmes `00` à `03` :

| Élément | Valeur vérifiée |
|---|---|
| Puce | ESP32-D0WD-V3 rev 3, 2 cœurs, 240 MHz |
| Mémoire | 4 Mo de flash, **pas de PSRAM** |
| USB série | CH340 (COM5 sur le PC de Dimitri) |
| Écran | pilote **ILI9341_2** (TFT_eSPI), **sans inversion**, rotation 1 = paysage 320 × 240, couleurs justes |
| Broches écran | MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, RST −1, rétroéclairage 21 (PWM) |
| Tactile | XPT2046 résistif, bus logiciel : CLK 25, MOSI 32, MISO 39, CS 33, IRQ 36 |
| Orientation tactile | axes échangés (brut Y → abscisse écran, brut X → ordonnée) |
| Port USB | sur le bord **droit** quand l'écran est en paysage |
| Lecture d'identifiant écran | impossible (renvoie 0) : normal sur cette carte |

Non vérifié ou douteux : LED RGB (GPIO 4/16/17, active à l'état bas, non confirmée visuellement),
capteur de lumière GPIO 34 (lectures 0 à 43, à retester), microSD, haut-parleur.
Pas de circuit de charge Li-Po sur la carte d'après les sources consultées ; pas de batterie prévue.

### Tactile : pièges déjà résolus
- Le XPT2046 change sa sortie sur le front descendant : **lire chaque bit pendant que l'horloge est haute**.
  Lire après le front descendant doublait toutes les valeurs (modulo 4096) et rendait le tactile inutilisable.
- Pression : `z = z1 + 4095 − z2`, seuil 200. Au repos le bruit monte jusqu'à ~72 ; les appuis
  légers près du bord droit donnent ~300.
- Calibration : correspondance **affine à 9 cibles** (`x = a·bx + b·by + c`, idem pour y), mesurée
  par `03-diag-tactile` : écart moyen 4,5 px, pire 10,8 px. Coefficients par défaut dans
  `02-interface/src/touch.cpp`. Recalibrer : maintenir l'écran au démarrage.
- Mesures : 2 lectures à blanc, médiane de 5, relâchement validé après 4 lectures vides.
- Un appui long sur un petit objet en bas de l'écran (le compagnon de 48 × 34 px) **n'a jamais
  fonctionné** malgré deux essais : ne pas compter dessus, préférer de gros boutons.

## 3. Logiciel

- PlatformIO, `platform = espressif32@6.9.0`, framework Arduino, carte `esp32dev`.
- Bibliothèques : `bodmer/TFT_eSPI@2.5.43` (configurée par `build_flags`, pas de User_Setup),
  `lvgl/lvgl@8.3.11` avec `LV_CONF_SKIP=1` (réglages par `build_flags`).
- **`LV_MEM_CUSTOM=1`** et tampon d'affichage alloué au démarrage : avec un bloc LVGL statique de 64 Ko,
  la RAM statique débordait (8 Ko de trop) après l'ajout du Wi-Fi.
- Polices : Outfit (texte) et IBM Plex Mono (chiffres), générées avec `lv_font_conv` en Latin-1
  pour avoir les accents français (`src/fonts/`). Les polices `font_num_*` ne contiennent que
  les chiffres, l'espace et « : ».
- Ne pas nommer de variables `TX` ou `RX` : déjà définies par le cœur Arduino ESP32.

### Dossiers (`firmware/`)
| Dossier | Rôle |
|---|---|
| `00-identification` | Test série : puce, mémoire, identifiants écran, tactile brut |
| `01-ecran` | Test d'affichage : variantes ILI9341 / ST7789 (ILI9341_2 retenu) |
| `03-diag-tactile` | Diagnostic tactile : pression, 9 cibles, écart, dessin libre |
| `02-interface` | **Le firmware clin** (voir ci-dessous) |

### Fichiers de `02-interface/src`
- `main.cpp` : écran, LVGL, rétroéclairage PWM, boucle (Wi-Fi + LVGL), fuseau Europe/Paris.
- `touch.cpp/.h` : lecture XPT2046, calibration affine, enregistrement en mémoire (Preferences « touch4 »).
- `wifi_mgr.cpp/.h` : 3 réseaux mémorisés (Preferences « wifi »), essais à tour de rôle toutes les 15 s
  sans bloquer l'interface, recherche des réseaux, NTP à la connexion.
- `ui.cpp/.h` : toute l'interface.
- `secrets.example.h` : modèle facultatif ; `secrets.h` est ignoré par git.

## 4. Ce que fait le firmware v0.5

- **Démarrage** (~3 s) : le compagnon ouvre les yeux, cligne, sourit ; « clin », « ton compagnon de bureau ».
- **Barre du bas** : Accueil, Focus, [compagnon animé], Vision, Trajets.
- **Accueil** : « Bonjour, Dimitri », vraie heure et date en français, nom du Wi-Fi en haut à droite,
  carte **Pointage** (dernier pointage) et carte **Options** (ouvre les Réglages).
  Toucher l'heure ouvre le réglage manuel.
- **Pointage** : gros boutons ARRIVÉE / DÉPART, **fenêtre de confirmation** (Confirmer / Annuler).
  Historique et Corriger : pas encore faits. Les pointages sont **en mémoire vive seulement**.
- **Focus** : cercle de progression, 25/5 ou 50/10, Démarrer / Pause / Reprendre / Arrêter,
  enchaînement automatique concentration → pause.
- **Vision** : 8 exercices (20-20-20, clignements, près/loin, suivi lent, points alternés, huit,
  respiration, au hasard), 20 à 90 s, écran sombre, bouton Arrêter, mention
  « Confort visuel, pas un avis médical ».
- **Trajets** : 3 temps **fictifs** (Domicile, Chez Ornella, Récupérer les enfants), marqués « fictif ».
- **Réglages** (via Options) : Wi-Fi (liste, clavier AZERTY 5 rangées, 3 réseaux mémorisés, « Oublier tout »),
  heure manuelle, emplacement réservé « Prochain projet ».
- **Heure** : NTP (pool.ntp.org) dès la connexion, sinon manuelle (« heure manuelle » affiché).
  Pas de pile : l'heure manuelle est perdue à la coupure.
- **Veille** : après 1 min sans toucher, si ni Focus ni exercice ne tourne, grand compagnon animé
  (cligne, regarde à gauche/droite, sourit, surpris), « clin » et l'heure, écran tamisé.
  **Écran éteint après 5 min** de veille. Un appui rallume sans déclencher de bouton.

## 5. Décisions prises par Dimitri (à respecter)

- Interface sombre bleu/cyan, gros boutons espacés (tactile résistif). Validée sur maquette puis sur la carte.
- **Alarme de la maison : retirée de l'application** (« elle n'a rien à faire dans l'application »).
- **Nombre de pas : retiré de l'accueil**, remplacé par la carte Options.
- Pas d'appui long pour ouvrir les réglages (ne fonctionnait pas) : passer par la carte Options.
- Confirmation obligatoire avant un pointage.
- Heure : NTP + réglage manuel de secours (pas de module RTC pour l'instant).
- Home Assistant : API REST avec jeton longue durée, à travers le tunnel Cloudflare (pas de Cloudflare Access).
- Interface en LVGL.
- Bluetooth non retenu pour Internet ; Wi-Fi seulement (box ou partage de connexion).
- Nom du projet : **clin**.

## 6. Reste à faire (ordre proposé)

1. Pointage enregistré en flash (LittleFS), historique, correction, export.
2. Mode hors ligne explicite et marquage « ancien » des données distantes avec l'heure de mise à jour.
3. Home Assistant : trajets réels (entités à fournir par Dimitri), notifications choisies,
   envoi des pointages avec identifiant unique et accusé de réception.
4. Luminosité réglable, réglage des durées de veille.
5. Boîtier imprimé (voir 7).
6. « Prochain projet » : bouton réservé dans Réglages, contenu non défini.

## 7. Boîtier

- Style retenu : **mini Echo Show 5** (forme inspirée, sans logo) : façade noire arrondie,
  fenêtre d'écran rectangulaire à coins vifs, corps incliné, trame « tissu », environ 106 × 62 × 68 mm.
- Posé ou collé au double face sous l'écran de l'ordinateur, légèrement incliné vers le visage,
  câble USB droit discret qui part derrière le moniteur. Port USB de la carte : bord droit.
- Imprimante : Bambu Lab X1C. **Aucun fichier d'impression n'est encore fait** : il manque
  les mesures exactes de la carte (dimensions, entraxes et diamètre des trous, épaisseur, zone visible).

## 8. Règles de travail

- Aucun secret dans git : mots de passe Wi-Fi, jetons Home Assistant, adresses, URL privées.
  Les réseaux Wi-Fi saisis à l'écran sont stockés en clair dans la flash de la carte.
- Ne jamais inventer d'entités Home Assistant, d'adresses ou de valeurs : les demander.
- Toute donnée distante affiche sa fraîcheur ; les valeurs de démonstration sont marquées « fictif ».
- Les exercices visuels sont du confort, pas un avis médical, et s'arrêtent immédiatement.
- Tester sur la carte réelle avant de figer un réglage matériel.
- L'environnement cloud de Claude ne peut pas joindre le registre PlatformIO : le code n'y est pas
  compilé. Dimitri compile et téléverse ; les erreurs reviennent par copie du terminal.
- Branche de travail GitHub : `ccr-61b7f5a0-iv25ff` du dépôt `squal32/squal32`.
- Garder cohérents ce fichier et la page Notion « Assistant de bureau ESP32 — contexte et plan de fabrication ».

## 9. Liens

- Aperçu des écrans de clin : https://claude.ai/artifact/F5pTP4qjZ9FhYHvEPLEHUm
- Maquette animée d'origine (8 scènes) : https://claude.ai/artifact/QWruAjfMmkw1qLM7dznDSj
- Concept 3D du boîtier : https://claude.ai/artifact/Y8BZagDe5e4Ec5Han5SrVu
- Vue éclatée du boîtier : https://claude.ai/artifact/4kd9A9AE2tC8U3Uxj7kwRP
(Pages privées : visibles seulement par Dimitri ou les personnes avec qui il les partage.)
