# Retro-Go SD — Music homebrew (GWHB).
#
#   make PROJECT_KIND=homebrew
#   make docker PROJECT_KIND=homebrew
#
# Drop Music.bin under /homebrews/ on the SD card. Tracks live in /music.

#######################################
# Project identity
#######################################
PROJECT_KIND ?= homebrew

CORE_NAME  := music
CORE_ENTRY := app_main

CORE_C_SOURCES := \
src/main.c \
src/hb_compat.c \
src/music/main_music.c \
src/music/music_audio.c \
src/music/music_cover.c \
src/music/music_id3.c \
src/music/music_lyrics.c \
src/music/music_ui.c \
src/music/music_minimp3.c \
src/music/music_lupng.c \
src/music/tjpgd.c \
src/music/progjpeg.c \
src/music/miniz.c

CORE_C_INCLUDES := \
-Isrc/include \
-Isrc/music

GNW_CORE_SDK ?= sdk
BUILD_DIR ?= build/$(PROJECT_KIND)

#######################################
# Kind-specific compile defs + packing
#######################################
ifeq ($(PROJECT_KIND),core)
$(error This project is homebrew-only; use PROJECT_KIND=homebrew)
else ifeq ($(PROJECT_KIND),homebrew)
CORE_C_DEFS := \
-DPROJECT_KIND_HOMEBREW=1

PACKED_BIN := Music.bin
COVER_JPG  := $(BUILD_DIR)/cover.jpg

else
$(error PROJECT_KIND must be 'homebrew' (got '$(PROJECT_KIND)'))
endif

include $(GNW_CORE_SDK)/Makefile

PACK_HOMEBREW := $(GNW_CORE_SDK)/tools/pack_homebrew.py

#######################################
# Pack
#######################################
.PHONY: pack cover

.PHONY: cover
cover: $(COVER_JPG)

$(COVER_JPG):
	@mkdir -p $(BUILD_DIR)
	python3 -c "from pathlib import Path; from PIL import Image, ImageDraw; \
img=Image.new('RGB', (186,100), (16,24,48)); \
d=ImageDraw.Draw(img); \
d.rectangle((8,8,177,91), outline=(220,200,120), width=2); \
d.text((48,38), 'Music', fill=(255,240,180)); \
img.save('$(COVER_JPG)', 'JPEG', quality=85, optimize=True); \
sz=Path('$(COVER_JPG)').stat().st_size; \
assert sz <= 10*1024, f'cover too big: {sz}'"

pack: $(TARGET_BIN) $(COVER_JPG)
	$(V)$(ECHO) [ PACK GWHB ] $(PACKED_BIN)
	$(V)python3 $(PACK_HOMEBREW) \
		--elf $(TARGET_ELF) --bin $(TARGET_BIN) \
		--name "Music" --version 1.0.0 \
		--cover $(COVER_JPG) \
		--out $(PACKED_BIN)

all: pack

.PHONY: print-PROJECT_KIND print-PACKED_BIN print-CORE_NAME print-DOCKER_IMAGE
print-PROJECT_KIND:
	@echo $(PROJECT_KIND)
print-PACKED_BIN:
	@echo $(PACKED_BIN)
print-CORE_NAME:
	@echo $(CORE_NAME)
print-DOCKER_IMAGE:
	@echo $(DOCKER_IMAGE)

clean::
	$(V)rm -f $(PACKED_BIN)
	$(V)rm -f $(COVER_JPG)

#######################################
# Docker
#######################################
.PHONY: docker docker_pull docker_shell

RELEASE_VERSION ?= v1.5
DOCKER_REPOSITORY ?= sylverb/retro-go-sd-builder
DOCKER_IMAGE ?= $(DOCKER_REPOSITORY):$(RELEASE_VERSION)

DOCKER_TTY_FLAG := $(shell if [ -t 0 ]; then echo -it; else echo; fi)
DOCKER_USER := $(shell id -u):$(shell id -g)
DOCKER_RUN := docker run --rm $(DOCKER_TTY_FLAG) \
	--user $(DOCKER_USER) \
	-v "$(CURDIR):/opt/workdir" \
	-w /opt/workdir \
	$(DOCKER_IMAGE)

docker:
	$(V)$(ECHO) "[ DOCKER ]" $(DOCKER_IMAGE) "PROJECT_KIND=$(PROJECT_KIND)"
	$(V)$(DOCKER_RUN) make --no-print-directory -j$$(nproc) PROJECT_KIND=$(PROJECT_KIND)

docker_pull:
	$(V)$(ECHO) "[ PULL ]" $(DOCKER_IMAGE)
	$(V)docker pull $(DOCKER_IMAGE)

docker_shell:
	$(DOCKER_RUN) bash
