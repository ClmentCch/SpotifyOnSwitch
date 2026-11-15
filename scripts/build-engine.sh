#!/usr/bin/env bash
# Everything Spotify's browser needs besides the NRO:
#
#   1. the Linux kernel as a library, for the PC bench (aarch64-linux-gnu);
#   2. the guest's libraries: EGL and GLES (the console's GPU), ALSA (the
#      console's sound) and the H.264 decoder of the console's video engine;
#   3. Chromium's root filesystem, as an ext4 image the browser mounts.
#
# Runs in the engine container (vendor/chromium-nx/gx/docker/Dockerfile: Debian
# 13 with the arm64 packages and qemu-user-static), or on any Debian/Ubuntu
# machine with the same packages installed:
#
#   scripts/fetch-kernel.sh
#   WORK=$PWD/.work scripts/build-engine.sh
#
# The NRO build (scripts/build-nro.sh) uses what is written here.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
WORK=${WORK:-$ROOT/.work}
export LINUX=${LINUX:-$WORK/linux}
export LKL=${LKL:-$LINUX/tools/lkl}
export OUT=${OUT:-$WORK/gx-out}
GX=$ROOT/vendor/chromium-nx/gx

[ -f "$LKL/Makefile" ] || { echo "no kernel at $LINUX: run scripts/fetch-kernel.sh" >&2; exit 1; }
[ -x /usr/bin/qemu-aarch64-static ] || { echo "qemu-user-static is missing" >&2; exit 1; }
[ -x /usr/lib/chromium/chromium ] || { echo "Debian's arm64 chromium is missing" >&2; exit 1; }

mkdir -p "$OUT"

echo "== the kernel, as a library for the PC bench"
bash "$GX/build-lkl.sh"

echo "== the guest's libraries"
NO_TESTS=1 bash "$GX/build-pc.sh"

echo "== Chromium's root filesystem"
NO_TESTS=1 bash "$GX/build-chromium.sh"

ls -la "$OUT"/chromium-full-switch.ext4 "$OUT"/profile.ext4.gz
