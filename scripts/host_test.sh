#!/usr/bin/env bash
# Builds the platform-independent parts of Musictt on the host and runs the
# checks in tests/host_test.c. Used by CI and by `make host-test`.
set -euo pipefail

cd "$(dirname "$0")/.."
OUT="${TMPDIR:-/tmp}/musictt-host-test"
mkdir -p "$OUT"

gcc -std=c11 -O1 -g -Wall -Wextra -Wno-unused-parameter \
    -I source -I vendor/cjson \
    source/library.c source/settings.c source/i18n.c source/color.c \
    vendor/cjson/cJSON.c tests/host_test.c \
    -o "$OUT/host_test"

"$OUT/host_test"
