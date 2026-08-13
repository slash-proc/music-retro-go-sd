# Music — Retro-Go SD Homebrew

GWHB homebrew MP3/WAV player for
[Game & Watch Retro-Go SD](https://github.com/sylverb/game-and-watch-retro-go-sd).

Browse `/music` on the SD card (fallback `/media`), play with the Winamp-style
deck (covers, lyrics, favourites, seek, volume, screen-off). Ported from the
jshsakura Music overlay; talks to the launcher only through `gw_firmware_abi_t`
(needs a firmware build that includes the Music/media ABI appends: PCM ops on
`audio_ctl`, `overlay_ctl` / `i18n_ctl` / `math_ctl`, backlight level on
`lcd_ctl`).

## Build

```bash
make PROJECT_KIND=homebrew
# or: make docker PROJECT_KIND=homebrew
```

Produces `Music.bin` → copy to `/homebrews/Music.bin` on the SD card.
Put tracks under `/music/` (`.mp3` / `.wav`). Favourites persist to
`/music/.favourites`.

## Layout

| Path | Role |
|------|------|
| `src/main.c` | `app_main` → `app_main_music` |
| `src/music/` | Player (UI, audio, ID3, covers, lyrics) + vendored minimp3/tjpgd/lupng/miniz |
| `src/hb_compat.c` | Local `rg_storage_scandir` + English strings + `draw_fill_rect` |
| `src/include/` | Music headers + thin stubs (`gui.h`, `rg_i18n.h`, …) |
| `sdk/` | Vendored ABI bridge (sync with `./scripts/sync_from_firmware.sh`) |

Third-party notices: [THIRD_PARTY.md](THIRD_PARTY.md).

## Requirements

- `arm-none-eabi-gcc` (hard-float `fpv5-d16`) or Docker image
  `sylverb/retro-go-sd-builder`
- Python 3 + Pillow for the cover JPEG
- Firmware flashed with ABI size ≥ this binary’s `required_abi_min_size`
  (see pack output)
