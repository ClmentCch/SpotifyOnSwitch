#!/usr/bin/env bash
# Checks what the build made before it is published:
#
#   * dist/Spotify.nro is a real NRO (magic NRO0), large enough to hold the
#     Linux kernel and the browser, and its NACP names it "Spotify";
#   * dist/spotify-engine.ext4 is an ext4 filesystem, the size of a browser;
#   * dist/manifest.txt lists both, with their real sizes and SHA-256.
#
# Run by the workflow after the build, and by hand after scripts/build-nro.sh.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
DIST=${DIST:-$ROOT/dist}

[ -f "$DIST/Spotify.nro" ] || { echo "dist/Spotify.nro is missing" >&2; exit 1; }
[ -f "$DIST/spotify-engine.ext4" ] || { echo "dist/spotify-engine.ext4 is missing" >&2; exit 1; }
[ -f "$DIST/manifest.txt" ] || { echo "dist/manifest.txt is missing" >&2; exit 1; }

python3 - "$DIST" <<'PY'
import hashlib, os, struct, sys

dist = sys.argv[1]
nro_path = os.path.join(dist, "Spotify.nro")
img_path = os.path.join(dist, "spotify-engine.ext4")
data = open(nro_path, "rb").read()

def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()

if data[:4] != b"NRO0":
    sys.exit("Spotify.nro: not an NRO (magic %r)" % data[:4])
size = len(data)
if size < 15 << 20:
    sys.exit("Spotify.nro: %d bytes, too small to hold the kernel and the browser" % size)

# The NACP (the app's metadata, which HBMenu reads) follows the NRO; its name
# is 0x200 bytes from 8 bytes past the "NACP" magic.
found = []
start = 0
while True:
    i = data.find(b"NACP", start)
    if i < 0 or i + 8 + 0x200 > size:
        break
    raw = data[i + 8:i + 8 + 0x200].split(b"\0")[0]
    try:
        found.append(raw.decode("utf-8"))
    except UnicodeDecodeError:
        pass
    start = i + 4
if "Spotify" not in found:
    sys.exit("Spotify.nro: the NACP does not name the app Spotify (found %r)" % (found[:4],))
if b"open.spotify.com/intl-fr/" not in data:
    sys.exit("Spotify.nro: the Spotify web player's address is not in the program")
if b"@spotify-engine.ext4 =profile.ext4" not in data:
    sys.exit("Spotify.nro: the engine image is not what the browser starts")

img = os.path.getsize(img_path)
if img < 100 << 20:
    sys.exit("spotify-engine.ext4: %d bytes, too small to be Chromium" % img)
with open(img_path, "rb") as f:
    f.seek(0x438)
    if f.read(2) != b"\x53\xef":
        sys.exit("spotify-engine.ext4: no ext4 magic at 0x438")

manifest = {}
for line in open(os.path.join(dist, "manifest.txt")):
    line = line.strip()
    if "=" in line:
        k, v = line.split("=", 1)
        manifest.setdefault(k, []).append(v)
if manifest.get("version") != ["1.0.0"]:
    sys.exit("manifest.txt: version %r" % manifest.get("version"))
files = {}
for entry in manifest.get("file", []):
    name, n, h = entry.split()
    files[name] = (int(n), h)
for name, path in (("Spotify.nro", nro_path), ("spotify-engine.ext4", img_path)):
    if name not in files:
        sys.exit("manifest.txt: %s is not listed" % name)
    n, h = files[name]
    if n != os.path.getsize(path):
        sys.exit("manifest.txt: %s is %d bytes, the manifest says %d" % (name, os.path.getsize(path), n))
    real = sha256(path)
    if real != h:
        sys.exit("manifest.txt: %s has SHA-256 %s, the manifest says %s" % (name, real, h))

print("Spotify.nro: %.1f MB, NRO0, NACP \"Spotify\"" % (size / 1048576.0))
print("spotify-engine.ext4: %.1f MB, ext4" % (img / 1048576.0))
print("manifest.txt: version 1.0.0, both files and their SHA-256 match")
PY
