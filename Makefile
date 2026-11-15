# Spotify for Nintendo Switch: a Chromium browser that opens the real Spotify
# web player, as an NRO for HBMenu.
#
# The build has two halves:
#   make engine   the web engine: LKL's kernel library for the PC bench, the
#                 guest's EGL/GLES/ALSA/H.264 libraries, and Chromium's root
#                 filesystem as an ext4 image (Debian 13 arm64 container);
#   make nro      Spotify.nro, linked against LKL for the console (devkitA64).
#
# `make docker` runs both in the containers the CI uses.

DEVKITPRO ?= /opt/devkitpro
WORK ?= $(CURDIR)/.work
DOCKER_ENGINE ?= spotify-engine
DOCKER_NX ?= devkitpro/devkita64

.PHONY: all nro engine kernel docker verify dist clean

all: nro

kernel:
	KERNEL_SHA=$${KERNEL_SHA:-d0f76a77e8c2ca68bd7d96190b8da09040eab072} \
		bash scripts/fetch-kernel.sh $(WORK)/linux

engine: kernel
	WORK=$(WORK) DEVKITPRO=$(DEVKITPRO) bash scripts/build-engine.sh

nro: kernel
	WORK=$(WORK) DEVKITPRO=$(DEVKITPRO) bash scripts/build-nro.sh

docker:
	bash scripts/build-docker.sh

verify:
	DIST=$(CURDIR)/dist bash scripts/verify.sh

clean:
	rm -rf $(WORK) dist vendor/chromium-nx/gx/dist-nx
