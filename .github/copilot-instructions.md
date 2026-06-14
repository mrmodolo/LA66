# Copilot instructions for LA66 embedded LoRa project

This file gives Copilot sessions focused, actionable pointers for building, navigating and changing this repository.

## Quick build / flash commands
- Prepare environment: source build/envsetup.sh (sets TREMO_SDK_PATH to repo root and adjusts PATH for arm toolchain when needed).
- Build the default application:
  - cd Projects/Applications/DRAGINO-LRWAN-AT && make
- Build + flash (device required):
  - cd Projects/Applications/DRAGINO-LRWAN-AT && make flash
  - Configure SERIAL_PORT and SERIAL_BAUDRATE in the Makefile or export them before running make.
- Keil project generation: cd Projects/Applications/DRAGINO-LRWAN-AT && make IDE=keil

Notes: the build system expects an ARM Cortex-M4 toolchain (arm-none-eabi-*). If missing, envsetup.sh attempts to add the bundled toolchain under tools/toolchain.

## Tests & linters
- No automated unit tests or linters are present at the repository root or the application project. There is no `make test` target. If adding tests, place them in the specific subproject and document the invocation in this file.

## High-level architecture (big picture)
- Top-level layout:
  - Drivers/: MCU and peripheral drivers used by applications
  - Middlewares/LoRa/: LoRa radio stack and LoRaMac-node implementation (the LoRaWAN protocol layer)
  - Projects/Applications/DRAGINO-LRWAN-AT/: The end-user application (firmware) — contains src/, inc/, cfg/ and a Makefile
  - build/: build system helpers (make includes, scripts, toolchain integration)
  - tools/: helper scripts and binary tools (flash utilities, keil project generator)
- Build model: project Makefiles assemble sources by wildcarding files relative to TREMO_SDK_PATH. Project-specific Makefile sets CFLAGS, DEFINES, INC paths and includes build/make/common.mk for toolchain/link rules.
- Runtime: Projects/Applications/DRAGINO-LRWAN-AT/src/main.c is the firmware entrypoint; it wires drivers, timers, radio and LoRaMac callbacks (LORA_RxData, LORA_HasJoined, etc.).

## Key repository conventions
- TREMO_SDK_PATH is authoritative: most Makefiles assume TREMO_SDK_PATH points to the SDK root. Use build/envsetup.sh to set it when working locally.
- Project sources are assembled via wildcards in each project Makefile; avoid editing common.mk—put project changes in the project Makefile.
- Linker scripts are per-project under cfg/ (e.g. cfg/gcc.ld). Output binaries and intermediate objects go to OUT_DIR (default: Make_out) — do not commit build artifacts.
- Flags and region settings: per-project defines (e.g., REGION_EU868, CONFIG_DEBUG_UART) live in the project Makefile; adjust there for region/feature changes.
- Flashing uses a Python loader script (build/scripts/tremo_loader.py) invoked by make flash; ensure SERIAL_PORT and SERIAL_BAUDRATE are correctly set.

## Files and places to inspect for common tasks
- To change radio or LoRa behavior: inspect Middlewares/LoRa and Projects/Applications/DRAGINO-LRWAN-AT/src/lora_app.c/.h
- To change hardware bindings: check Drivers/peripheral/src and Projects/Applications/DRAGINO-LRWAN-AT/inc
- Build toolchain and flags: build/make/common.mk and build/make/toolchain_arm-none-eabi.mk

## AI-assistant / other assistant configs to merge
- No CLAUDE.md, .cursorrules, .clinerules, .windsurfrules, or AGENTS.md were found in the repo root. If added later, include key build/setup lines here.

---

If anything in these instructions should be expanded (examples for common edits, cross-reference to specific functions, or adding a test/lint workflow), say which area to cover and Copilot will add it.
