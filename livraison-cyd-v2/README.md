# CYD miniature — V2 à corps galbé

Cette version reprend la silhouette des références fournies : façade à contours arrondis, corps profond aux angles adoucis, trois boutons sur le dessus et finition textile à coller. Elle conserve l'écran CYD 2,8 pouces au format 4:3 et une fenêtre à coins droits. Les bordures latérales restent larges : la carte 86 × 50 mm est nettement plus grande que sa zone active 57,6 × 43,2 mm. Un écran large comme celui de l'appareil de référence demanderait une autre carte/une autre dalle.

Enveloppe nominale : 110 × 64 × 55 mm ; hauteur réelle de la coque seule 64,434 mm, hors poussoirs. Façade inclinée de 10°. La profondeur de 55 mm remplace volontairement les 25 mm de V1 pour retrouver la forme demandée. La V1 reste disponible dans son dossier d'origine.

## Vue éclatée : huit pièces à imprimer

| Repère | Fichier STL | Quantité | Rôle |
| --- | --- | --- | --- |
| 1 | face.stl | 1 | Façade noire brillante, contour extérieur arrondi, fenêtre 58 × 44 mm à angles vifs |
| 2 | carrier.stl | 1 | Support amovible du PCB, entraxes 78 × 42 mm |
| 3 | body.stl | 1 | Coque galbée, paroi nominale 2,4 mm, à recouvrir de tissu fin |
| 4 | rear.stl | 1 | Dos plat 98 × 58 mm, fixation par quatre vis |
| 5 | button_minus.stl, button_mute.stl, button_plus.stl | 3 | Poussoirs ; symbole cercle pour muet |
| 6 | diffuser.stl | 1 | Diffuseur de la barre lumineuse |

L'écran avec son PCB jaune est représenté uniquement par une enveloppe mécanique. Il n'est pas à imprimer et n'est pas inclus dans les STL. Les vis, le tissu, les interrupteurs et leur support, la source LED et le câblage sont à ajouter. La texture textile et la brillance du rendu sont des finitions ; elles ne sont pas inscrites dans le maillage STL. Aucun logo, aucune donnée privée ni aucun secret ne sont incorporés.

## Impression FDM

Buse de 0,4 mm, couche de 0,2 mm, 3 à 4 périmètres, remplissage 20 à 30 %. PETG conseillé. Ne pas redimensionner globalement les pièces : les entraxes et jeux sont déjà en millimètres.

- Façade : face visible contre un plateau lisse ; épaisseur 2 mm. Pour le brillant, employer un matériau/plateau adaptés ou effectuer une finition par ponçage et vernis compatible.
- Support PCB : grande face plane à Z=0 contre le plateau, entretoises vers le haut.
- Coque : ouverture arrière vers le plateau ; dans le trancheur, abaisser sa face arrière de Z=2 à Z=0. Des supports locaux peuvent être nécessaires sous les quatre pattes internes, les passages latéraux et les trous horizontaux. Préférer des supports peints localement et inspecter le tranchage ; leur absence n'a pas été validée par une impression.
- Dos : face de collage plane à Z=0 contre le plateau, fraisures vers le plateau. Garder cette face lisse.
- Poussoirs : collerette Ø8 contre le plateau. Tige Ø5,2 dans un perçage Ø5,8, soit 0,3 mm de jeu radial. Un mécanisme de rappel et des interrupteurs restent nécessaires.
- Diffuseur : semelle 39 × 4 mm contre le plateau. Languette 35,4 × 1,2 dans une ouverture 36 × 1,8 mm, soit 0,3 mm de jeu par côté. Matériau translucide conseillé.

## Montage

1. Essayer la carte réelle sur le support. Les quatre centres sont x=16/94 et y=11,5/53,5 dans le repère du support. Les entretoises ont des avant-trous borgnes Ø1,8. Employer de petites vis pour plastique Ø2 mm avec rondelles isolantes adaptées aux trous du PCB Ø3,2. Limiter la pénétration à environ 3 mm, sans serrer le PCB sous contrainte.
2. Visser le support sur les quatre pattes de la coque : passages Ø2,4, avant-trous Ø1,8, axes x=8/102 et y=16/49 dans le repère incliné de la façade. Longueur de vis à sélectionner pour environ 3 mm de prise après traversée du support.
3. Installer les trois poussoirs par l'intérieur et prévoir le support des interrupteurs, le rappel et les fils. Ne pas coller les parties mobiles. Le modèle ne fournit pas de fonction électrique volume/muet.
4. Coller le diffuseur par sa semelle intérieure. Ajouter une source lumineuse compatible : la LED intégrée au PCB n'éclaire pas automatiquement cette fenêtre.
5. Présenter la façade sans la coller. Vérifier l'accès au tactile, les connecteurs et les jeux. Coller ensuite la façade sur le bord avant de la coque avec une colle compatible, sans déborder dans la zone active. L'entretien se fait par le dos.
6. Fermer le dos par quatre vis fraisées Ø2 mm, longueur indicative 8 mm à confirmer suivant la tête choisie. Les centres sont x=10/100 et y=9/55 ; la tête ne doit pas dépasser la face extérieure.
7. Zone de double face entièrement plane : x=13 à 97, y=8 à 56, soit 84 × 48 mm. Elle ne recouvre pas les fraisures. La tenue de l'adhésif sur votre moniteur reste à tester.

## Ports, variantes et limites

La position des connecteurs et l'épaisseur des composants de la variante Guition à deux USB ne sont pas certifiées par le plan d'origine. Les découpes latérale et supérieure sont des accès de prototype volontairement larges. Vérifier leur alignement et la course d'éjection microSD sur la carte réelle avant finition textile.

Les USB de la carte documentée sont sur un petit côté. La sortie en haut et à l'arrière de la coque permet le passage d'un câble ou d'un report USB à ajouter. Elle ne transforme pas une prise latérale du PCB en prise supérieure ; le report, son support et la décharge de traction doivent être adaptés au câble choisi. La fiche USB droite peut rester branchée latéralement en attendant.

Pas de batterie, circuit de charge ni haut-parleur spécifiés dans cette version. Le schéma de la carte d'origine ne permet pas d'affirmer que votre variante possède un chargeur Li-Po. Les rendus ne représentent pas les composants arrière ni les câbles.

## Contrôles effectués

Les huit STL sont fermés, correctement orientés, à volume positif et chacun d'un seul tenant. Les intersections contrôlées entre coque, façade, support, dos, poussoirs, diffuseur et enveloppe du PCB restent sous 0,05 mm³ ; les résidus de contact de l'ordre de 0,003 mm³ proviennent de la précision des maillages. Cela ne valide pas l'encombrement de composants non modélisés, les efforts mécaniques, l'impression ni l'ajustement à la carte réelle. Voir validation.json.

Les images assemblée et éclatée sont calculées à partir des STL, avec une enveloppe indicative de la carte. Les déplacements de la vue éclatée servent à montrer les pièces, pas à définir la course d'assemblage.

## Régénération

```sh
cd /workspace/cyd-mini-v2
bash export.sh
/workspace/cyd-tools/bin/python verify.py
blender -b -t 6 --python render_blender.py
/workspace/cyd-tools/bin/python annotate.py
```

Outils utilisés : OpenSCAD 2021.01, Blender 4.3.2, Python 3.12, trimesh 4.12.2, numpy 2.4.2, scipy 1.17.0, manifold3d 3.3.2, Pillow 12.1.0. Les images ne nécessitent pas de service web.

## Sources des cotes de carte

- [Plan d'origine de la CYD : 86 × 50, entraxes 78 × 42, trous Ø3,2](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/0564a140c76de76ef34a8b7115044e7c6c852695/OriginalDocumentation/3-Structure_Diagram/Dimensions.png).
- [Fiche d'origine, zone active 57,6 × 43,2](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/0564a140c76de76ef34a8b7115044e7c6c852695/OriginalDocumentation/2-Specification/ESP32-2432S028%20Specifications-EN.pdf).

Ces cotes sont celles de la carte documentée ; elles ne remplacent pas l'identification de la variante physique. La forme du nouveau boîtier est un dessin original établi à partir des références visuelles fournies.
