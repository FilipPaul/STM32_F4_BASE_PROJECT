# CMake vs Make: Runtime Difference Notes

Date: 2026-06-28
Project: F4_WITH_ETHERNET

## Goal
Document the concrete build-system differences that can cause the firmware to run correctly with Make but fail when built with CMake.

## 1) Source Set Differences
Primary risk: different compiled source files change application behavior.

### CMake includes custom application sources
Defined in top-level CMake:
- `Core/Src/commandParser.c`
- `Core/Src/EEPROM_24AA02E48T_I_IOT_settings.c`
- `Core/Src/globals.c`
- `Core/Src/I2C_PCA9548APWR_expander.c`
- `Core/Src/IPsettings.c`
- `Core/Src/multiplexers.c`
- `Core/Src/RAFION_bitbang_I2C.c`
- `Core/Src/relayCards.c`
- `Core/Src/RELAYS_AND_IO_DRIVER.c`
- `Core/Src/shiftRegisters.c`
- `Core/Src/tcpecho.c`
- `Core/Src/tcpserver.c`
- `Core/Src/UartMessageHandler.c`
- `Core/Src/UsbMessageHanler.c`
- `Core/Src/VL53L1X_RAFIONTECH.c`

CMake also pulls STM32CubeMX-generated source groups through `cmake/stm32cubemx/CMakeLists.txt`.

### Attached Makefile source list differs
The attached `Makefile` mainly contains generated CubeMX sources and does not show the custom app source set above.

Impact:
- Different initialization paths
- Different global state / IRQ interactions
- Different network/runtime behavior

## 2) Optimization and Debug Macro Differences
Primary risk: timing and conditional code paths differ.

### CMake
File: `cmake/gcc-arm-none-eabi.cmake`
- Debug flags: `-O0 -g3`
- Adds `DEBUG` define in Debug config through CubeMX CMake (`$<$<CONFIG:Debug>:DEBUG>`)

### Attached Makefile
File: `Makefile`
- Debug enabled: `DEBUG = 1`
- Optimization: `OPT = -Og`
- No explicit `-DDEBUG` in `C_DEFS` (only `USE_HAL_DRIVER`, `STM32F407xx`)

Impact:
- ISR/timing sensitivity can change between `-O0` and `-Og`
- `#ifdef DEBUG` sections may compile in CMake but not in Make

## 3) Linker and Library Differences
Primary risk: memory layout and runtime support differences.

### CMake
File: `cmake/gcc-arm-none-eabi.cmake`
- Linker script: `STM32F407XX_FLASH.ld`
- Toolchain libraries: `m`
- Linker flags include `--specs=nano.specs`, map file, gc-sections, print-memory-usage

### Attached Makefile
File: `Makefile`
- Linker script: `STM32F407XX_FLASH.ld`
- Libraries: `-lc -lm -lnosys`
- Linker flags include `-specs=nano.specs`, map file, gc-sections

Impact:
- Runtime/syscall/newlib behavior can differ if library set differs in final link

## 4) Important Note About STM32-for-VSCode Make Path
The flashed command in terminal uses:
- `make -f STM32Make.make flash`

`STM32Make.make` differs from attached `Makefile` in key ways:
- Uses linker script `STM32F407ZGTx_FLASH.ld` (configured)
- Adds `-u _printf_float`
- Uses `-Og -g -gdwarf -ggdb` in debug

This means there are effectively three build definitions in the repo:
1. `Makefile` (Cube-generated)
2. `STM32Make.make` (extension-generated; currently used for flashing)
3. CMake (`CMakeLists.txt` + toolchain + `cmake/stm32cubemx`)

## Recommended Alignment Order
1. Decide which Make baseline is the source of truth (`Makefile` or `STM32Make.make`).
2. Make CMake compile exactly the same source list as the chosen baseline.
3. Match optimization/debug defines (`-Og` vs `-O0`, and `DEBUG` macro policy).
4. Match linker script and linker flags (`_printf_float`, libs, nano/newlib behavior).
5. Rebuild, flash, and verify behavior.

## Next Action
Apply a minimal CMake patch that mirrors the selected working Make baseline 1:1.
