#!/usr/bin/env bash
# The Linux kernel LKL builds on top of, at the revision this project is
# pinned to: the same tree makes the PC bench's library and the console's.
#
#   scripts/fetch-kernel.sh [target-dir]      (default: .work/linux)
#
# Nothing is fetched when the directory is already a checkout of that
# revision. Chromium NX builds against LKL's master; a fixed commit makes a
# build repeatable.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
KERNEL_URL=${KERNEL_URL:-https://github.com/lkl/linux.git}
KERNEL_SHA=${KERNEL_SHA:-d0f76a77e8c2ca68bd7d96190b8da09040eab072}
TARGET=${1:-$ROOT/.work/linux}

if [ -e "$TARGET/tools/lkl/liblkl.a" ] || [ -e "$TARGET/tools/lkl/lib/lkl.o" ]; then
	echo "kernel already built: $TARGET"
	exit 0
fi
if [ -d "$TARGET/.git" ]; then
	have=$(git -C "$TARGET" rev-parse HEAD 2>/dev/null || true)
	[ "$have" = "$KERNEL_SHA" ] && { echo "kernel already at $KERNEL_SHA"; exit 0; }
fi

echo "fetching $KERNEL_URL at $KERNEL_SHA"
rm -rf "$TARGET"
mkdir -p "$TARGET"
git -C "$TARGET" init -q
git -C "$TARGET" remote add origin "$KERNEL_URL"
git -C "$TARGET" fetch -q --depth 1 origin "$KERNEL_SHA"
git -C "$TARGET" checkout -q FETCH_HEAD
echo "kernel at $(git -C "$TARGET" rev-parse --short HEAD) ($TARGET)"
