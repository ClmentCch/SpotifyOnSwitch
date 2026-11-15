#!/usr/bin/env bash
# The whole build, in the two containers the CI uses (needs Docker, and an
# amd64 machine that can run arm64 binaries: on Linux, qemu-user-static and
# binfmt-support installed on the host).
#
#   scripts/build-docker.sh
#
# Writes dist/Spotify.nro and dist/spotify-engine.ext4.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
DEVKITA64=${DEVKITA64:-devkitpro/devkita64}
VOLUME=${VOLUME:-spotify-work}

cd "$ROOT"
docker build -t spotify-engine -f vendor/chromium-nx/gx/docker/Dockerfile vendor/chromium-nx/gx/docker

docker rm -f spotify-engine-run spotify-nx >/dev/null 2>&1 || true
docker run -d --name spotify-engine-run -v "$ROOT:/project" -v "$VOLUME:/work" spotify-engine sleep infinity
trap 'docker rm -f spotify-engine-run spotify-nx >/dev/null 2>&1 || true' EXIT
docker exec spotify-engine-run bash /project/scripts/fetch-kernel.sh /work/linux
docker exec -e WORK=/work spotify-engine-run bash /project/scripts/build-engine.sh

docker run -d --name spotify-nx -v "$ROOT:/project" -v "$VOLUME:/work" "$DEVKITA64" sleep infinity
docker exec spotify-nx apt-get update
docker exec spotify-nx apt-get install -y --no-install-recommends \
	flex bison bc libelf-dev cpio rsync build-essential
docker exec -e WORK=/work spotify-nx bash /project/scripts/build-nro.sh

DIST="$ROOT/dist" bash scripts/verify.sh
