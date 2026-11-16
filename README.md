# Musictt

<img src="assets/musictt.png" width="128" alt="Logo Musictt">

**Lecteur musical natif pour Nintendo Switch.** Musictt est une application
homebrew au format NRO (libnx + SDL2) : l'interface, le scan de la carte SD et
le décodage audio sont exécutés par la console. Ce n'est ni un site web, ni une
WebView, ni une maquette.

| | |
|---|---|
| Application | **Musictt** |
| Fichier | **`Musictt.nro`** (atmosphère + Homebrew Menu) |
| Installation | `sd:/switch/Musictt/Musictt.nro` |
| Données | `sd:/MUSIC/` |
| Formats audio | MP3, WAV, FLAC, OGG (Vorbis) |
| Langues | 12, choisies au premier lancement |

## Fonctionnalités

- **Premier démarrage** : écran de bienvenue, 12 langues avec drapeaux dessinés,
  défilement/navigation à la manette et au tactile, bouton **Confirmer**, choix
  conservé dans `settings.json`.
- **Préparation de la carte SD** : création de `PLAYLISTS/`, `LOGO/` et
  `CONFIG/` sans jamais toucher aux fichiers existants, écran de confirmation
  puis écran « quitter / relancer » pour laisser le temps d'ajouter des fichiers.
- **Bibliothèque** : scan de `sdmc:/MUSIC/PLAYLISTS/`, un sous-dossier = une
  playlist, un `package.json` décrit la playlist et ses morceaux. Aucune
  playlist n'est codée en dur. Bouton d'actualisation partout.
- **Accueil** : bienvenue, playlists avec pochettes, morceaux récemment
  écoutés, informations de bibliothèque.
- **Playlists** : grille de playlists (pochette, nom, nombre de morceaux).
- **Page playlist** : grande pochette, nom, nombre de morceaux, liste complète
  avec pochette individuelle, titre et **bouton triangle** de lecture à droite.
- **Écran de lecture** : pochette centrée à coins arrondis, fond dégradé calculé
  à partir des **couleurs dominantes** de la pochette, titre, playlist, barre de
  progression réellement tactile (déplacement dans le morceau), temps courant et
  durée, précédent / lecture-pause / suivant.
- **Lecture continue** : la musique continue quand on revient en arrière ou que
  l'on navigue dans les autres pages (mini-lecteur persistant). En fin de
  morceau, passage automatique au suivant, avec pochette, titre, fond et
  progression mis à jour.
- **Lecteur audio natif** : décodage MP3 (dr_mp3), WAV (dr_wav), FLAC (dr_flac)
  et OGG (stb_vorbis), sortie 48 kHz stéréo via SDL2/audout, volume avec rampe
  anti-clic, recherche précise au échantillon près.
- **Paramètres** : langue, volume, animations, actualisation, à propos. Tout est
  persisté dans `sd:/MUSIC/CONFIG/settings.json`, y compris la dernière playlist,
  le dernier morceau et les écoutes récentes.
- **Erreurs gérées** : carte SD absente, dossier `PLAYLISTS` manquant, JSON
  invalide, playlist vide, fichier audio absent, format non pris en charge,
  fichier trop long, pochette absente (image de remplacement), sortie audio
  indisponible. Messages traduits, jamais de plantage.

## Installation

1. Télécharger `Musictt.nro` depuis la [release](../../releases).
2. Créer `sd:/switch/Musictt/` et y copier le fichier.
3. Lancer le Homebrew Menu (**maintenir R** sur un jeu) puis **Musictt**.

Le premier lancement ne charge volontairement aucune bibliothèque : l'écran
final invite à quitter Musictt, à ajouter les fichiers dans `MUSIC` puis à le
relancer.

## Structure de la carte SD

```
sdmc:/MUSIC/
├── PLAYLISTS/
│   ├── MANGER/
│   │   ├── package.json
│   │   ├── 1.mp3
│   │   └── 2.mp3
│   └── SPORT/
│       ├── package.json
│       └── 1.mp3
├── LOGO/
│   ├── playlist_manger.png
│   ├── music_manger_1.png
│   └── music_sport_1.png
└── CONFIG/
    └── settings.json      (écrit par Musictt)
```

`package.json` minimal :

```json
{
  "id": "2.1",
  "name": "Manger",
  "playlist_logo": "playlist_manger.png",
  "songs": [
    {
      "id": "2.1.1",
      "number": 1,
      "name": "Musique numéro 1",
      "file": "1.mp3",
      "logo": "music_manger_1.png"
    }
  ]
}
```

- `playlist_logo` et `logo` sont des **noms de fichiers** résolus dans
  `sdmc:/MUSIC/LOGO/` (les chemins avec `/` ou `..` sont refusés).
- `file` est un nom de fichier résolu dans le dossier de la playlist.
- `id` doit être unique et stable ; `number` donne la position du morceau.
- Sans `logo`, Musictt cherche automatiquement `music_<playlist>_<numéro>.png` ;
  sans `playlist_logo`, `playlist_<playlist>.png`. Sinon, une image de
  remplacement est affichée.

Le [guide des playlists](GUIDE-PLAYLISTS.md) détaille toutes les règles.

## Commandes

| Action | Tactile | Manette |
|---|---|---|
| Naviguer | toucher | stick gauche / croix |
| Valider | toucher | **A** |
| Retour | flèche retour | **B** (ou **Échap** au clavier) |
| Lecture / pause | bouton central | **X** |
| Actualiser | icône ↻ | **Y** |
| Pages | barre latérale | **L** / **R** |
| Volume | curseur | **ZL** / **ZR** |
| Écran de lecture | mini-lecteur | **Start** |

La barre de progression accepte un appui n'importe où puis un glissement ; le
volume se règle de la même façon.

## Compilation

Prérequis : [devkitPro](https://devkitpro.org/) avec devkitA64, libnx et les
portlibs SDL2.

```bash
dkp-pacman -S switch-sdl2 switch-sdl2_image switch-sdl2_ttf
make -j$(nproc)
```

Le `Makefile` produit `Musictt.nro` (titre NACP « Musictt », auteur, icône
`assets/icon.jpg`). Les polices Noto Sans et Noto Sans CJK sont liées dans la
section `.rodata` : aucun fichier supplémentaire n'est nécessaire sur la carte
SD.

`.github/workflows/build.yml` exécute la même compilation dans l'image
`devkitpro/devkita64` épinglée, lance les tests hôtes, vérifie le NRO et publie
la release sur un tag `v*`. Le registre des étapes est dans le dépôt.

Tests rapides (sans console) :

```bash
make host-test
```

Ils couvrent le scan des playlists, la résolution des pochettes, les identifiants
uniques, les erreurs JSON, la persistance des réglages, la présence des 12
traductions et le calcul de couleur dominante.

## Limites connues

- **Pas de test sur console physique** : le NRO est réellement compilé et
  vérifié (format NRO, NACP, icône, chaînes), mais le rendu, le tactile et la
  sortie audio n'ont pas pu être validés sur une Switch.
- Un morceau est décodé entièrement en mémoire avant lecture (recherche exacte,
  pas de coupure). Au-delà d'environ 18 minutes en 44,1 kHz stéréo, il est
  refusé avec un message clair (budget mémoire de 192 Mio).
- Les métadonnées ID3 ne sont pas lues : titres et pochettes viennent des
  `package.json`.
- Rendu logique en 1280×720, mis à l'échelle par la console en mode docké.
- Pas de lecture réseau, de streaming ni de paroles : uniquement des fichiers
  locaux sur la carte SD.

## Licence

Code Musictt sous licence MIT (voir [LICENSE](LICENSE)). Les polices, décodeurs
et bibliothèques tierces gardent leurs licences respectives ; voir
[NOTICE.md](NOTICE.md) et [assets/OFL-Noto.txt](assets/OFL-Noto.txt).
