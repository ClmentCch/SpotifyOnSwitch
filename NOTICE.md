# Third-party notices

Musictt is MIT-licensed (see `LICENSE`). It embeds or links the following
components, each under its own licence.

## Fonts

- **Noto Sans** and **Noto Sans CJK** — Copyright 2014-2024 Adobe
  (http://www.adobe.com/), with Reserved Font Name 'Source'.
  Licensed under the SIL Open Font License, Version 1.1.
  Full text: `assets/OFL-Noto.txt`.
  Files: `assets/NotoSans-Regular.ttf`, `assets/MusicttCJK.ttc`.

## Audio decoders

- **dr_mp3**, **dr_wav**, **dr_flac** — Copyright 2016-2024 David Reid.
  Public domain (or MIT-0 where public domain is not recognised).
  `vendor/dr/`.
- **stb_vorbis** — Copyright 2007-2024 Sean Barrett and contributors.
  Public domain / MIT. `vendor/stb/stb_vorbis.c`.

## JSON

- **cJSON** — Copyright 2009-2017 Dave Gamble and cJSON contributors.
  MIT licence. `vendor/cjson/`.

## SDK and libraries (linked, not vendored)

- **libnx** — Copyright 2017-2024 libnx contributors. ISC licence.
- **SDL2**, **SDL2_image**, **SDL2_ttf** — Copyright 1997-2024 Sam Lantinga
  and contributors. zlib licence.
- **FreeType** — Copyright 1996-2024 The FreeType Project. FreeType licence
  (FTL) or GPLv2, at your option.
- **HarfBuzz** — Copyright 2010-2024 Behdad Esfahbod and contributors. MIT.
- **libpng** — PNG Reference Library licence.
- **libjpeg-turbo** — IJG / BSD-3-Clause / zlib.
- **libwebp** — Copyright 2010 Google Inc. BSD-3-Clause.

The build links these from devkitPro's portlibs; their sources are available
from devkitPro and their respective upstream projects.
