# Provenance du moteur

`vendor/chromium-nx/` est le code source de
**[shodowlo/chromium-nx](https://github.com/shodowlo/chromium-nx)**, version
`0.1.0`, commit `87a8095d20cf73b0fde0eee3215e70ebc887f048`, licence **GPL-2.0**
(voir `LICENSE` dans ce dossier), adapté pour Spotify.

Chromium NX fait tourner du Linux (LKL, GPL-2.0) comme bibliothèque dans un
NRO, et exécute dessus le Chromium arm64 de Debian, non modifié, avec le GPU,
le son et le tactile de la console.

## Ce qui a été modifié

Modifications apportées par ce dépôt, toutes dans ce dossier :

- **Identité** : nom « Spotify », NACP `Spotify`, sortie `Spotify.nro`, icône
  `assets/icon.jpg` (dérivée du logo officiel), dossier d'installation
  `sd:/switch/Spotify`.
- **Page d'accueil** : `https://open.spotify.com/intl-fr/`, plein écran, sans
  barre d'adresse (`-F`, `-A0,0,0,0`), moteur `spotify-engine.ext4`.
- **Démarrage** : écran de démarrage noir/vert avec le logo, téléchargement
  automatique du moteur au premier lancement, puis ouverture du site sans
  passer par l'écran d'accueil du lanceur.
- **Commandes** : le stick gauche déplace le pointeur, **A** clique, **B**
  revient, **X** met en lecture/pause (touche média), **Y** ouvre le clavier,
  **L/R** défilent, **−** actualise, **+** ouvre le panneau.
- **Mises à jour** : dépôt et fichiers de release pointés sur
  `ClmentCch/SpotifyOnSwitch`.
- **Couleurs** : `#121212` et `#1DB954` dans le lanceur et le panneau.
- Les chemins de données (`profile.ext4`, `Downloads/`, journal) suivent le
  nouveau dossier.

Ce qui n'a **pas** été modifié : le noyau LKL, l'exécuteur ELF (`gelf.c`,
`gmem.c`, `gsys.c`, `tramp.S`), le compositeur Wayland (`wl.c`), le rendu
(`glwin.c`), le décodeur vidéo, le réseau (NAT), et les sources de Chromium
(ce sont les paquets arm64 de Debian, non modifiés).

## Licence

L'ensemble forme une œuvre dérivée sous GPL-2.0. Le texte de la licence est
dans `LICENSE`, à la racine du dépôt et dans ce dossier. Les sources complètes
nécessaires à la reconstruction sont dans ce dépôt, et les étapes de
compilation sont dans `scripts/` et `.github/workflows/build.yml`.
