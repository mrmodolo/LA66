# Building LA66 on Linux (Ubuntu/Mint/Debian)

This document describes the changes required to successfully build the Dragino LA66 firmware on modern Linux distributions such as Ubuntu 24.04 and Linux Mint 22.

## Tested Environment

* Distribution: Linux Mint 22.3 (Zena)
* Base: Ubuntu 24.04 (Noble)
* GCC: arm-none-eabi-gcc 13.2.1
* Target: Dragino LA66 USB Adapter V1.1
* Firmware: DRAGINO-LRWAN-AT

## Install Dependencies

```bash
sudo apt update
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi make python3
```

Verify installation:

```bash
arm-none-eabi-gcc --version
```

Expected output:

```text
arm-none-eabi-gcc (15:13.2.rel1-2) 13.2.1
```

## Fix 1: Replace stdint-gcc.h

Modern ARM GCC versions no longer provide `stdint-gcc.h`.

Replace:

```c
#include <stdint-gcc.h>
```

with:

```c
#include <stdint.h>
```

Affected file:

```
Drivers/system/printf-stdarg.c
```

## Fix 2: Include stdint.h

Some headers assume `stdint.h` is already included by the toolchain.

Add:

```c
#include <stdint.h>
```

to:

```
Drivers/sensor/flash_eraseprogram.h
```

## Fix 3: Case-sensitive filesystem issues

The original project appears to have been developed on Windows. Linux filesystems are case-sensitive.

Correct the include path:

Replace:

```make
$(TREMO_SDK_PATH)/Middlewares/LoRa/system/crypto
```

with:

```make
$(TREMO_SDK_PATH)/Middlewares/LoRa/system/Crypto
```

in:

```
Projects/Applications/DRAGINO-LRWAN-AT/Makefile
```

Also verify that:

```make
$(TREMO_SDK_PATH)/Drivers/crypto/inc
```

uses the correct case matching the actual directory structure.

## Build

Enter the application directory:

```bash
cd Projects/Applications/DRAGINO-LRWAN-AT
```

Compile using:

```bash
make TREMO_SDK_PATH=$(realpath ../../..)
```

Successful output should generate:

```
Make_out/DRAGINO-LRWAN-AT.elf
Make_out/DRAGINO-LRWAN-AT.bin
```

Example:

```text
Build completed.
```

## Flashing

After compilation, the SDK suggests the following command:

```bash
python3 ../../../build/scripts/tremo_loader.py \
    -p /dev/ttyUSB0 \
    -b 921600 \
    flash 0x08000000 \
    Make_out/DRAGINO-LRWAN-AT.bin
```

### Bootloader mode

To enter bootloader mode:

1. Disconnect the USB adapter.
2. Connect BOOT to GND.
3. Reconnect USB.
4. Press RESET (if available).
5. Execute the flashing command.
6. Remove the BOOT jumper.
7. Reset the device again.

## AU915 (Brazil)

To build firmware for Brazil, modify:

```
Projects/Applications/DRAGINO-LRWAN-AT/Makefile
```

Change:

```make
-DREGION_EU868
```

or:

```make
-DREGION_AS923
```

to:

```make
-DREGION_AU915
```

After flashing, configure the appropriate AU915 sub-band for your LoRaWAN network server (TTN, ChirpStack, etc.).

## Notes

These modifications do not alter the firmware functionality. They only address compatibility issues between the original SDK and modern Linux environments.

