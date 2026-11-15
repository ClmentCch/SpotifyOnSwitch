# The engine's build container: Debian 13 with the arm64 packages Chromium
# runs from, and qemu-user to inspect them from this amd64 machine.
#
#   docker build -t spotify-engine -f vendor/chromium-nx/gx/docker/Dockerfile \
#       vendor/chromium-nx/gx/docker
#
# Chromium 156 is Debian's own arm64 build: the browser is not compiled here,
# only put in the ext4 image the console mounts (gx/build-chromium.sh).
FROM debian:trixie

ENV DEBIAN_FRONTEND=noninteractive
RUN dpkg --add-architecture arm64
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential gcc-aarch64-linux-gnu libc6-dev-arm64-cross qemu-user-static \
        flex bison bc libelf-dev python3 git make rsync cpio ca-certificates file \
        curl:arm64 \
        libegl1:arm64 libgles2:arm64 libegl-mesa0:arm64 libgl1-mesa-dri:arm64 \
        libegl-dev:arm64 libgles-dev:arm64 \
        chromium:arm64 chromium-headless-shell:arm64 \
        fonts-dejavu-core e2fsprogs libxkbcommon-tools xkb-data \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /project
