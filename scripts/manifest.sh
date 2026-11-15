#!/usr/bin/env bash
# Writes dist/manifest.txt: what the app reads from its release to know which
# engine image goes with this NRO, and whether a newer version exists
# (gx/launcher/update.c). The format is the one Chromium NX publishes.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
DIST=${DIST:-$ROOT/dist}
VERSION=$(sed -n 's/^#define GX_VERSION "\(.*\)"$/\1/p' "$ROOT/vendor/chromium-nx/gx/version.h")
TAG="v$VERSION"

[ -f "$DIST/Spotify.nro" ] || { echo "dist/Spotify.nro is missing" >&2; exit 1; }

python3 - "$DIST" "$VERSION" "$TAG" <<'PY'
import hashlib, os, sys

dist, version, tag = sys.argv[1], sys.argv[2], sys.argv[3]
lines = ["version=" + version, "tag=" + tag]
for name in ("Spotify.nro", "spotify-engine.ext4"):
    path = os.path.join(dist, name)
    if not os.path.exists(path):
        sys.exit("%s is missing" % name)
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    lines.append("file=%s %d %s" % (name, os.path.getsize(path), h.hexdigest()))
open(os.path.join(dist, "manifest.txt"), "w", newline="\n").write("\n".join(lines) + "\n")
print("\n".join(lines))
PY
