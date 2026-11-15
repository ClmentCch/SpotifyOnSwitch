<p align="center">
  <img src="assets/Spotifylogo.png" alt="Spotify" width="180">
</p>

<h1 align="center">Spotify</h1>

<p align="center">
  Le site officiel Spotify Web, en plein écran, sur une Nintendo Switch.
</p>

---

**Spotify** est une application homebrew pour Nintendo Switch, au format `.nro`,
qui ouvre le **véritable site Spotify Web** — `https://open.spotify.com/intl-fr/` —
dans un vrai moteur **Chromium 156** exécuté par la console.

Ce n'est pas une imitation : aucune interface HTML ne simule Spotify, aucun
lecteur factice n'est embarqué, et le site n'est pas modifié. Le moteur web est
Debian's Chromium (arm64), lancé par le noyau Linux compilé en bibliothèque
(LKL) à l'intérieur du NRO, avec le rendu GPU, le son et le tactile de la
console.

- **Application :** Spotify
- **Fichier :** `Spotify.nro`
- **Dossier d'installation :** `sd:/switch/Spotify/`

## Sommaire

- [Fonctionnalités](#fonctionnalités)
- [Compatibilité](#compatibilité)
- [Installation](#installation)
- [Utilisation](#utilisation)
- [Contrôles Joy-Con](#contrôles-joy-con)
- [Écran tactile](#écran-tactile)
- [Connexion Spotify](#connexion-spotify)
- [Compilation depuis les sources](#compilation-depuis-les-sources)
- [Limitations connues](#limitations-connues)
- [Structure du dépôt](#structure-du-dépôt)
- [Remerciements](#remerciements)
- [Licence](#licence)

## Fonctionnalités

- Le vrai Spotify Web, chargé depuis `open.spotify.com/intl-fr/`.
- Plein écran : pas de barre d'adresse, pas de barre d'onglets, pas
  d'interface de navigateur autour de la page.
- Écran de démarrage aux couleurs de Spotify (`#121212`, `#1DB954`) avec le
  logo officiel, puis lancement automatique du site.
- Joy-Con : pointeur au stick, clic, retour, lecture/pause, défilement,
  actualisation, panneau de réglages.
- Écran tactile complet : toucher, glisser, pincer pour zoomer.
- Clavier virtuel de la console pour les champs de recherche et de connexion.
- Session conservée d'un démarrage à l'autre (cookies, connexion), dans
  `sd:/switch/Spotify/profile.ext4`.
- Téléchargement du moteur web au premier démarrage, puis mise à jour depuis
  les releases de ce dépôt.
- Messages d'erreur explicites : pas de connexion, erreur DNS ou TLS, moteur
  absent, téléchargement interrompu, avec réessai.

## Compatibilité

| | |
| --- | --- |
| Console | Nintendo Switch (Erista, Mariko, OLED, Lite) |
| CFW | Atmosphère, via HBMenu (nx-hbmenu) |
| Format | NRO (homebrew), aucune installation système |
| Mémoire | **lancer en mode « titre »** (voir ci-dessous) : Chromium a besoin de ~1,9 Go |
| Firmware | celui de votre Atmosphère ; aucun titre système n'est modifié |
| Réseau | Wi-Fi de la console, HTTPS vers GitHub et Spotify |

Chromium ne tient pas dans la mémoire d'un applet (le HBMenu ouvert depuis
l'album). Pour disposer de la mémoire complète, **maintenez R en lançant un jeu
ou une application installée** : HBMenu s'ouvre alors en mode titre. Un
forwarder créé avec [Sphaira](https://github.com/Team-Neptune) (espace
d'adressage 39 bits) donne le même résultat depuis le menu HOME.

## Installation

1. Télécharger **`Spotify.nro`** depuis la page
   [Releases](https://github.com/ClmentCch/SpotifyOnSwitch/releases).
2. Créer le dossier `Spotify` dans `sd:/switch/` s'il n'existe pas.
3. Placer le fichier ici :

   ```
   sd:/switch/Spotify/Spotify.nro
   ```

4. Ouvrir HBMenu (en mode titre : maintenir **R** au lancement d'un jeu).
5. Lancer l'application nommée exactement **Spotify**.

### Premier démarrage

Le moteur web (`spotify-engine.ext4`, ~420 Mo) n'est pas dans le NRO : il est
téléchargé une seule fois, au premier démarrage, depuis la release du projet,
et enregistré dans `sd:/switch/Spotify/`. L'écran de démarrage affiche la
progression ; une erreur réseau laisse un bouton **A — Réessayer**.

Pour l'installer hors ligne, télécharger `spotify-engine.ext4` depuis la
release et le copier dans `sd:/switch/Spotify/` à côté du NRO.

Contenu final du dossier :

```
sd:/switch/Spotify/
├── Spotify.nro            l'application
├── spotify-engine.ext4    le moteur web (téléchargé au premier démarrage)
├── profile.ext4           votre session (créé au premier lancement)
├── settings.ini           les réglages (créé au premier lancement)
└── spotify.log            le journal de la dernière exécution
```

## Utilisation

Au démarrage : logo Spotify sur fond noir, vérification du moteur, puis
ouverture du site en plein écran. Connectez-vous avec votre compte Spotify
(formulaire du site officiel), ou utilisez la connexion Google si le moteur
l'accepte.

Le bouton **+** ouvre le panneau de l'application par-dessus la page : plein
écran, orientation, mode des commandes, clavier, presse-papiers, retour au
lanceur ou fermeture.

## Contrôles Joy-Con

Mode par défaut (**Browser**) : le stick gauche déplace un pointeur, **A**
clique.

| Bouton | Action |
| --- | --- |
| Stick gauche | déplacer le pointeur |
| **A** | cliquer / sélectionner |
| **B** | retour (page précédente) |
| **X** | lecture / pause (touche média de la page) |
| **Y** | clavier virtuel (saisie dans le champ sélectionné) |
| **Haut / Bas / Gauche / Droite** | élément précédent / suivant (navigation spatiale) |
| **L / R** | défiler vers le haut / le bas |
| **+** | paramètres de l'application (panneau) |
| **−** | actualiser la page |
| Stick droit | défilement fin |

Le panneau **+** propose aussi le mode *Keyboard & mouse*, où chaque bouton
devient une touche (clavier, souris, clic droit ZL / clic gauche ZR),
modifiable dans les réglages du lanceur, et le mode *Controller*, qui expose la
manette aux pages qui utilisent la Gamepad API.

## Écran tactile

L'écran tactile est celui de Chromium : un doigt est un doigt sur la page.

- toucher pour cliquer, glisser pour faire défiler ;
- deux doigts pour pincer et zoomer ;
- sélection des morceaux, ouverture des playlists et des albums ;
- champs de recherche et de connexion : le clavier virtuel de la console
  s'ouvre quand un champ de texte est activé ;
- lecteur : les commandes du site répondent au toucher.

## Connexion Spotify

La connexion se fait **sur le site officiel**, dans le moteur Chromium :
identifiants Spotify, ou connexion Google si le fournisseur l'autorise dans ce
navigateur. L'application n'intercepte rien, ne lit aucun mot de passe et ne
contourne aucune protection : elle affiche la page, c'est tout.

La session (cookies, jetons) est stockée localement dans
`sd:/switch/Spotify/profile.ext4`. Elle survit aux redémarrages de la console
et se supprime avec **Clear browsing data** dans les réglages du lanceur.
Aucune donnée de session n'est envoyée à GitHub ni à ce dépôt.

Si la connexion Google est refusée (« navigateur non sécurisé »), c'est une
décision du fournisseur, non contournable ; la connexion par identifiants
Spotify reste disponible.

## Compilation depuis les sources

Le projet se compile en deux temps, dans les conteneurs que la CI utilise :
Debian 13 arm64 pour le moteur, devkitA64 pour le NRO.

```bash
git clone https://github.com/ClmentCch/SpotifyOnSwitch
cd SpotifyOnSwitch
scripts/build-docker.sh          # → dist/Spotify.nro, dist/spotify-engine.ext4
```

Ou, sans Docker, sur une machine Debian/Ubuntu avec les paquets arm64, qemu-user,
et devkitPro installé :

```bash
scripts/fetch-kernel.sh          # le noyau LKL, révision épinglée
scripts/build-engine.sh          # bibliothèque noyau, EGL/GLES/ALSA, image Chromium
DEVKITPRO=/opt/devkitpro scripts/build-nro.sh
scripts/verify.sh
```

`make engine`, `make nro`, `make docker` et `make verify` enveloppent ces
étapes. Les versions sont épinglées (commit du noyau, digest de l'image
devkitA64, `debian:trixie`) pour que la compilation soit reproductible.

### Intégration continue

[`.github/workflows/build.yml`](.github/workflows/build.yml) :

1. installe devkitPro (conteneur officiel `devkitpro/devkita64`) et les
   paquets arm64 ;
2. construit le noyau Linux/LKL et les bibliothèques du moteur ;
3. fabrique l'image Chromium (Debian arm64, Chromium 156) ;
4. compile **`Spotify.nro`** ;
5. vérifie le NRO (magic `NRO0`, NACP « Spotify », URL du site, image du
   moteur), l'image ext4 et le manifeste, SHA-256 à l'appui ;
6. publie les fichiers en artifacts, puis, sur un tag `v*`, crée la release
   **Spotify v…** avec `Spotify.nro`, `spotify-engine.ext4` et `manifest.txt`.

Aucun NRO factice n'est produit : le fichier publié est celui que le
compilateur et l'éditeur de liens viennent de fabriquer.

## Limitations connues

- **Lecture audio et DRM.** Le lecteur web de Spotify utilise les *Encrypted
  Media Extensions* et exige **Widevine**, le module DRM propriétaire de
  Google. Debian's Chromium ne l'inclut pas et sa licence interdit sa
  redistribution ; cette version ne peut donc pas le fournir. L'interface, la
  connexion, la recherche et la navigation fonctionnent, mais la lecture des
  morceaux peut afficher « Playback of protected content is not enabled ».
  Contourner ce module n'est ni prévu ni autorisé.
- **Projet expérimental.** Le moteur (Chromium NX) est récent ; des plantages
  restent possibles sur les pages lourdes, et une seule page à la fois est
  utilisée.
- **Mémoire.** En applet (HBMenu depuis l'album), Chromium n'a pas assez de
  mémoire : lancer en mode titre ou par forwarder 39 bits.
- **Aucun test sur console physique pour cette version.** Les fichiers publiés
  sont réellement compilés et vérifiés en intégration continue ; le
  comportement à l'exécution (rendu, tactile, Joy-Con, audio) n'a pas été
  observé sur une Switch pendant cette mise au point. Les retours d'essai sont
  bienvenus dans les issues du dépôt.
- **Pas d'installation système** : c'est un NRO, il ne remplace aucun titre et
  ne demande aucun privilège particulier.

## Structure du dépôt

```
SpotifyOnSwitch/
├── Spotifylogo.png              le logo officiel fourni (inchangé)
├── assets/
│   ├── Spotifylogo.png          le même logo, pour la documentation
│   └── icon.jpg                 l'icône 256×256 du NRO, dérivée du logo
├── scripts/
│   ├── fetch-kernel.sh          le noyau LKL, révision épinglée
│   ├── build-engine.sh          noyau + bibliothèques + image Chromium
│   ├── build-nro.sh             Spotify.nro
│   ├── manifest.sh              manifest.txt (tailles, SHA-256)
│   ├── verify.sh                vérifications avant publication
│   └── build-docker.sh          les deux conteneurs, comme la CI
├── vendor/chromium-nx/          le moteur, GPL-2.0 (voir UPSTREAM.md)
│   ├── gx/                      LKL, exécuteur ELF, Wayland, overlay, lanceur
│   ├── nro/                     la couche Horizon (libnx) du NRO
│   └── LICENSE                  GPL-2.0
├── .github/workflows/build.yml  compilation et release automatiques
├── Makefile
├── README.md
└── LICENSE
```

## Remerciements

- **[shodowlo/chromium-nx](https://github.com/shodowlo/chromium-nx)** : le
  moteur, sans lequel ce projet n'existerait pas. C'est lui qui fait tourner
  Chromium sur Horizon OS, avec le rendu GPU, le son, le tactile et les
  manettes.
- **[mnnobre/lkl-switch](https://github.com/mnnobre/lkl-switch)** : Linux
  comme bibliothèque dans un NRO.
- [LKL](https://github.com/lkl/linux), [libnx](https://github.com/switchbrew/libnx),
  [devkitPro](https://devkitpro.org), [Mesa](https://www.mesa3d.org),
  [Chromium](https://www.chromium.org).
- Spotify et son logo appartiennent à Spotify AB. Ce projet n'est ni affilié à
  Spotify, ni à Nintendo, ni à Google.

## Licence

**GPL-2.0** : le NRO contient le noyau Linux (LKL), sous GPL-2.0, et le code de
Chromium NX, publié sous GPL-2.0. Voir [LICENSE](LICENSE) et
[vendor/chromium-nx/UPSTREAM.md](vendor/chromium-nx/UPSTREAM.md) pour la
provenance et la liste des modifications.

Chromium est distribué sous licence BSD ; les fichiers arm64 utilisés ici sont
ceux de Debian, non modifiés.
