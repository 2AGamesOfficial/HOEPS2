# HOEPS2

PS2 homebrew co-op arena shooter. Based on "Hell on Earth" (2020 game jam), ported to PlayStation 2.

## Controls
- Left stick - move
- Right stick - aim
- R1/R2 - shoot
- D-pad left/right - switch weapon
- Start - pause menu
- Select (both pads) - toggle team-kill
- P2 Start - join as Player 2

## Play
Copy HOEPS2/bin contents to USB or memcard. Boot via FMCB/OPL/wLE, or load the ELF in PCSX2.

## Build (Docker)
docker compose up -d
docker exec -t tyra-compiler sh -c "cd HOEPS2 && make clean && make build-engine all"

## Layout
- HOEPS2/ - game source, assets, built ELF
- engine/ - modified Tyra (see HOEPS2/bin/LICENSES.txt for changes)

## Credits
See HOEPS2/bin/LICENSES.txt.

(c) 2026 2A Games
