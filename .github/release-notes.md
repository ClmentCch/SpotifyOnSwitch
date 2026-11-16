## Musictt — lecteur musical natif pour Nintendo Switch

Une application homebrew **native** (NRO, libnx + SDL2), pas une page web : le
décodage audio et l'interface tournent directement sur la console.

### Installation

1. Télécharger **`Musictt.nro`** ci-dessous.
2. Le placer dans `sd:/switch/Musictt/Musictt.nro`.
3. Lancer le Homebrew Menu (maintenir **R** sur un jeu) et démarrer **Musictt**.

Au premier lancement, Musictt demande la langue, crée
`sd:/MUSIC/PLAYLISTS`, `sd:/MUSIC/LOGO` et `sd:/MUSIC/CONFIG`, puis propose de
quitter pour ajouter des fichiers sur la carte SD.

### Structure de la carte SD

```
sdmc:/MUSIC/
├── PLAYLISTS/
│   └── MANGER/
│       ├── package.json
│       ├── 1.mp3
│       └── 2.mp3
├── LOGO/
│   ├── playlist_manger.png
│   ├── music_manger_1.png
│   └── music_manger_2.png
└── CONFIG/
    └── settings.json
```

Exemple de `package.json` :

```json
{
  "id": "2.1",
  "name": "Manger",
  "playlist_logo": "playlist_manger.png",
  "songs": [
    { "id": "2.1.1", "number": 1, "name": "Musique numéro 1", "file": "1.mp3", "logo": "music_manger_1.png" }
  ]
}
```

Le guide complet est dans `GUIDE-PLAYLISTS.md` du dépôt.

### Fonctionnalités

- 12 langues réelles (français, anglais, espagnol, allemand, italien,
  portugais, japonais, coréen, chinois, russe, néerlandais, polonais), choisies
  au premier lancement et mémorisées.
- Scan des playlists de la carte SD, lecture des `package.json`, pochettes de
  playlist et de morceau, repli automatique sur une image de remplacement.
- Lecteur audio natif : MP3, WAV, FLAC et OGG, lecture/pause, précédent,
  suivant, volume, barre de progression tactile avec déplacement dans le
  morceau, enchaînement automatique.
- Écran de lecture plein écran avec fond dégradé calculé à partir des couleurs
  dominantes de la pochette.
- Interface tactile et manette (Joy-Con, manette Pro), barre latérale, accueil,
  playlists, bibliothèque, paramètres, mini-lecteur persistant.

### Limitations connues

- **Non testé sur console physique** dans le cadre de cette publication : le
  NRO et l'image sont compilés et vérifiés, mais le rendu, le tactile et la
  sortie audio doivent être confirmés sur une vraie Switch.
- Chaque morceau est décodé entièrement en mémoire avant lecture (recherche
  précise, aucune coupure). Les fichiers d'environ plus de 18 minutes en
  44,1 kHz stéréo sont refusés avec un message explicite.
- Les titres et pochettes viennent exclusivement des `package.json` : les tags
  ID3 intégrés aux fichiers ne sont pas lus.
- Rendu en 1280×720, mis à l'échelle par la console en mode docké.
- Seuls MP3, WAV, FLAC et OGG sont décodés ; les autres formats sont refusés
  avec un message traduit.

### Licence

Code Musictt sous MIT. Polices Noto sous SIL Open Font License 1.1, décodeurs
dr_libs/stb dans le domaine public, cJSON sous MIT, SDL2/SDL_image/SDL_ttf
sous zlib. Voir `NOTICE.md`.
