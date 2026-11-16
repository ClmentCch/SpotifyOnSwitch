# Guide des playlists Musictt

Musictt lit la carte SD, jamais le réseau. Une playlist est un dossier de
`sdmc:/MUSIC/PLAYLISTS/` contenant un `package.json` et des fichiers audio.

## 1. Créer une playlist

```
sdmc:/MUSIC/PLAYLISTS/
└── MANGER/
    ├── package.json
    ├── 1.mp3
    ├── 2.mp3
    └── 3.mp3
```

Le nom du dossier sert d'identifiant de secours, de nom de playlist et de base
aux conventions de pochettes. Les majuscules, espaces et accents sont acceptés.

## 2. Écrire `package.json`

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
    },
    {
      "id": "2.1.2",
      "number": 2,
      "name": "Musique numéro 2",
      "file": "2.mp3",
      "logo": "music_manger_2.png"
    }
  ]
}
```

### Champs

| Champ | Obligatoire | Rôle |
|---|---|---|
| `id` | non | identifiant stable de la playlist (sinon le nom du dossier) |
| `name` | non | nom affiché (sinon le nom du dossier) |
| `playlist_logo` | non | fichier de `MUSIC/LOGO/` |
| `songs` | oui | tableau des morceaux (peut être vide) |
| `songs[].id` | non | identifiant stable du morceau (sinon `<id playlist>.<numéro>`) |
| `songs[].number` | non | position dans la playlist (sinon l'ordre du tableau) |
| `songs[].name` | non | titre affiché |
| `songs[].file` | oui | nom du fichier audio dans le dossier de la playlist |
| `songs[].logo` | non | fichier de `MUSIC/LOGO/` |

Règles vérifiées par Musictt :

- Les identifiants doivent être **uniques**. Un doublon est renommé
  automatiquement (`x`, `x.2`) pour rester stable et utilisable.
- Les morceaux sont triés par `number`, puis par nom.
- `file`, `playlist_logo` et `logo` sont des **noms de fichiers** : les chemins
  contenant `/`, `\` ou `..` sont refusés.
- Un morceau dont le fichier est absent reste listé, avec un avertissement, et
  ne peut pas être lu.

## 3. Pochettes

Toutes les images vont dans `sdmc:/MUSIC/LOGO/` :

| Type | Convention | Exemple |
|---|---|---|
| Playlist | `playlist_NOM.png` | `playlist_manger.png` |
| Morceau | `music_NOM_NUMERO.png` | `music_manger_1.png` |

`NOM` est le nom du dossier en minuscules. Musictt utilise ces conventions
automatiquement quand `playlist_logo` / `logo` sont absents, puis bascule sur une
image de remplacement. Les formats PNG et JPEG sont acceptés ; les images sont
redimensionnées et mises en cache (aucune déformation : recadrage centré).

## 4. Formats audio

- MP3
- WAV (PCM)
- FLAC
- OGG Vorbis (`.ogg`, `.oga`)

L'extension fait foi. Un format non pris en charge affiche un message traduit.
Le morceau est décodé en mémoire avant lecture : au-delà d'environ 18 minutes en
44,1 kHz stéréo, Musictt refuse le fichier avec un message explicite plutôt que
de manquer de mémoire.

## 5. Actualiser

Après avoir ajouté ou renommé des fichiers, appuyez sur l'icône **↻** (ou **Y** à
la manette, ou « Actualiser la bibliothèque » dans les paramètres). Musictt ne
supprime ni ne modifie jamais vos fichiers : il ne lit que `package.json`, les
audios et les pochettes.

## 6. Dépannage

| Symptôme | Cause probable |
|---|---|
| « Aucune playlist détectée » | dossiers absents ou sans `package.json` valide |
| « package.json invalide » | virgule en trop, guillemets manquants, JSON commenté |
| Pochette grise | fichier absent de `LOGO/` ou nom incorrect |
| Morceau grisé | `file` introuvable ou extension non prise en charge |
| Bibliothèque vide au premier lancement | normal : Musictt ne scanne pas avant l'écran « relancer » |

Un exemple complet, prêt à copier, se trouve dans
[`examples/MUSIC`](examples/MUSIC) du dépôt.
