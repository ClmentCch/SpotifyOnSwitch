#!/usr/bin/env bash
# Checks that Musictt.nro is a real libnx NRO and that the app's own strings
# made it into the binary. Fails loudly rather than shipping a fake file.
set -euo pipefail
cd "$(dirname "$0")/.."

[ -f Musictt.nro ] || { echo "Musictt.nro was not produced" >&2; exit 1; }

python3 - <<'PY'
import os, sys

path = "Musictt.nro"
data = open(path, "rb").read()
size = len(data)

# libnx's elf2nro: branch, "HOMEBREW" at 8, "NRO0" at 0x10, NACP as the last
# 0x4000 bytes with the title starting at its first byte.
if data[8:16] != b"HOMEBREW" or data[16:20] != b"NRO0":
    sys.exit("Musictt.nro: not an NRO (header %r)" % data[:32])
if size < 5 << 20:
    sys.exit("Musictt.nro: %d bytes, too small to contain the player" % size)
if b"\xff\xd8\xff" not in data:
    sys.exit("Musictt.nro: no JPEG icon is embedded")

nacp = data[-0x4000:]
name = nacp[:0x200].split(b"\0")[0].decode("utf-8", "replace")
if name != "Musictt":
    sys.exit("Musictt.nro: the NACP names the app %r" % name)
if b"ClmentCch" not in nacp[:0x2400]:
    sys.exit("Musictt.nro: the NACP has no author")

required = [
    b"sdmc:/MUSIC",
    b"PLAYLISTS",
    b"package.json",
    b"playlist_logo",
    b"MP3",
]
for needle in required:
    if needle not in data:
        sys.exit("Musictt.nro: %r is missing from the binary" % needle)

print("Musictt.nro: %.1f MB, NRO0, NACP %r, icon and library strings present"
      % (size / (1024 * 1024), name))
PY

ls -la Musictt.nro
