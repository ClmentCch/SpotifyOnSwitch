#!/usr/bin/env bash
# Builds Spotify.nro and lays out what the release holds in dist/.
#
# In the console's build container (devkitpro/devkita64), with the engine
# built by scripts/build-engine.sh in the shared volume:
#
#   docker exec -e WORK=/work spotify-nx bash /project/scripts/build-nro.sh
#
# Or on a machine with devkitA64 and the Switch port libraries installed:
#
#   DEVKITPRO=/opt/devkitpro scripts/build-nro.sh
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
WORK=${WORK:-$ROOT/.work}
export LINUX=${LINUX:-$WORK/linux}
export PC_OUT=${PC_OUT:-$WORK/gx-out}
export OUT=${OUT:-$WORK/gx-nx}
export DEVKITPRO=${DEVKITPRO:-/opt/devkitpro}
export PATH=$DEVKITPRO/devkitA64/bin:$DEVKITPRO/tools/bin:$PATH

[ -f "$PC_OUT/profile.ext4.gz" ] || {
	echo "the engine is not built: run scripts/build-engine.sh first" >&2
	exit 1
}
[ -d "$LINUX/tools/lkl" ] || {
	echo "no kernel at $LINUX: run scripts/fetch-kernel.sh" >&2
	exit 1
}
command -v elf2nro >/dev/null || { echo "elf2nro is missing (devkitPro)" >&2; exit 1; }

echo "== the NRO"
LINUX=$LINUX OUT=$OUT PC_OUT=$PC_OUT bash "$ROOT/vendor/chromium-nx/gx/build-nx.sh"

mkdir -p "$ROOT/dist"
cp "$OUT/Spotify.nro" "$ROOT/dist/Spotify.nro"
if [ -f "$PC_OUT/chromium-full-switch.ext4" ]; then
	cp "$PC_OUT/chromium-full-switch.ext4" "$ROOT/dist/spotify-engine.ext4"
fi
bash "$ROOT/scripts/manifest.sh"
