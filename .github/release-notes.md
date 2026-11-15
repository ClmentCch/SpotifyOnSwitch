## Spotify pour Nintendo Switch

Le site officiel **Spotify Web** (`open.spotify.com`), en plein écran, dans un
vrai moteur Chromium exécuté par la console.

### Installation

1. Télécharger **`Spotify.nro`** ci-dessous.
2. Créer le dossier `Spotify` dans `sd:/switch/` s'il n'existe pas.
3. Y placer :

   ```
   sd:/switch/Spotify/Spotify.nro
   ```

4. Ouvrir HBMenu et lancer **Spotify**.

Au premier démarrage, l'application télécharge son moteur web
(`spotify-engine.ext4`, environ 420 Mo) depuis cette release et l'enregistre
dans `sd:/switch/Spotify/`. Ce téléchargement n'a lieu qu'une fois ; il peut
aussi être fait à la main en copiant `spotify-engine.ext4` dans le même
dossier.

### Ce qu'elle fait

- le vrai Spotify Web, pas une imitation : `https://open.spotify.com/intl-fr/` ;
- plein écran, sans barre d'adresse ni interface de navigateur ;
- Joy-Con et écran tactile ;
- clavier virtuel de la console pour les champs de recherche ;
- session conservée d'un démarrage à l'autre (cookies et connexion) ;
- mise à jour depuis les releases de ce dépôt.

### Limitations connues

- **Lecture audio :** le lecteur web de Spotify exige **Widevine**, le module
  DRM propriétaire de Google, présent dans Google Chrome mais pas dans le
  Chromium de Debian (sa licence interdit sa redistribution). L'interface, la
  connexion et la navigation fonctionnent ; la lecture des morceaux peut
  afficher « Playback of protected content is not enabled ».
- Projet basé sur Chromium NX, logiciel expérimental : quelques plantages
  restent possibles sur les pages lourdes.
- Aucun test sur console physique n'a été effectué pour cette version : elle
  est compilée et vérifiée en intégration continue, pas exécutée sur une
  Switch.

### Contenu de la release

| Fichier | Rôle |
| --- | --- |
| `Spotify.nro` | l'application (obligatoire) |
| `spotify-engine.ext4` | le moteur web, téléchargé au premier démarrage |
| `manifest.txt` | versions, tailles et SHA-256 des deux fichiers |
| `SHA256SUMS` | sommes de contrôle |

Licence GPL-2.0 (le noyau Linux/LKL et Chromium NX qu'elle embarque).
