# bootstrap.cmake
#
# Sets up everything the Makefile expects but doesn't provide itself:
# a verified ARM toolchain, the CMSIS repos, the startup files, and the
# linker script. Run this once after cloning the repo:
#
#   cmake -P bootstrap.cmake
#
# This is intentionally a script (-P), not a CMake *project* -- nothing
# here compiles anything, so there's no build/ directory or CMakeCache
# to clean up afterward. The actual firmware build stays entirely in
# the hand-written Makefile, unchanged.

message(STATUS "== Checking toolchain ==")

find_program(ARM_GCC arm-none-eabi-gcc)
if(NOT ARM_GCC)
    message(FATAL_ERROR
        "arm-none-eabi-gcc not found.\n"
        "  Debian/Ubuntu/Pop!_OS: sudo apt install gcc-arm-none-eabi\n"
        "  Otherwise: download the ARM GNU toolchain from developer.arm.com "
        "and add its bin/ to your PATH.")
endif()
message(STATUS "Found arm-none-eabi-gcc: ${ARM_GCC}")

find_program(OPENOCD_BIN openocd)
if(NOT OPENOCD_BIN)
    message(WARNING
        "openocd not found -- you won't be able to flash to real hardware.\n"
        "  Install it with: sudo apt install openocd\n"
        "  (Not required just to build or run static analysis, e.g. in CI.)")
else()
    message(STATUS "Found openocd: ${OPENOCD_BIN}")
endif()

find_program(GIT_EXECUTABLE git)
if(NOT GIT_EXECUTABLE)
    message(FATAL_ERROR "git not found. Install it with: sudo apt install git")
endif()

message(STATUS "== Fetching CMSIS ==")

# Matches the paths the Makefile already expects ($(HOME)/cmsis_core,
# $(HOME)/cmsis_device_g4) -- no Makefile changes needed.
set(CMSIS_CORE_DIR   "$ENV{HOME}/cmsis_core")
set(CMSIS_DEVICE_DIR "$ENV{HOME}/cmsis_device_g4")

if(NOT EXISTS "${CMSIS_CORE_DIR}/CMSIS/Core/Include/core_cm4.h")
    message(STATUS "Cloning cmsis_core into ${CMSIS_CORE_DIR}")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" clone --depth 1
                https://github.com/STMicroelectronics/cmsis_core
                "${CMSIS_CORE_DIR}"
        RESULT_VARIABLE CLONE_RESULT)
    if(NOT CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone cmsis_core")
    endif()
else()
    message(STATUS "cmsis_core already present, skipping")
endif()

if(NOT EXISTS "${CMSIS_DEVICE_DIR}/Include/stm32g474xx.h")
    message(STATUS "Cloning cmsis_device_g4 into ${CMSIS_DEVICE_DIR}")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" clone --depth 1
                https://github.com/STMicroelectronics/cmsis_device_g4
                "${CMSIS_DEVICE_DIR}"
        RESULT_VARIABLE CLONE_RESULT)
    if(NOT CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone cmsis_device_g4")
    endif()
else()
    message(STATUS "cmsis_device_g4 already present, skipping")
endif()

message(STATUS "== Populating startup/ ==")

set(STARTUP_DIR "${CMAKE_CURRENT_LIST_DIR}/startup")
file(MAKE_DIRECTORY "${STARTUP_DIR}")

if(NOT EXISTS "${STARTUP_DIR}/startup_stm32g474xx.s")
    file(COPY_FILE
        "${CMSIS_DEVICE_DIR}/Source/Templates/gcc/startup_stm32g474xx.s"
        "${STARTUP_DIR}/startup_stm32g474xx.s")
    message(STATUS "Copied startup_stm32g474xx.s")
else()
    message(STATUS "startup_stm32g474xx.s already present, skipping")
endif()

if(NOT EXISTS "${STARTUP_DIR}/system_stm32g4xx.c")
    file(COPY_FILE
        "${CMSIS_DEVICE_DIR}/Source/Templates/system_stm32g4xx.c"
        "${STARTUP_DIR}/system_stm32g4xx.c")
    # -nostartfiles still pulls in libgcc's __libc_init_array, which
    # expects _init/_fini to exist -- see the rest of this conversation
    # for why. ST's template doesn't define them, so we append stubs.
    file(APPEND "${STARTUP_DIR}/system_stm32g4xx.c"
         "\nvoid _init(void) {}\nvoid _fini(void) {}\n")
    message(STATUS "Copied system_stm32g4xx.c and appended _init/_fini stubs")
else()
    message(STATUS "system_stm32g4xx.c already present, skipping")
endif()

message(STATUS "== Writing linker script ==")

set(LINKER_DIR "${CMAKE_CURRENT_LIST_DIR}/linker")
file(MAKE_DIRECTORY "${LINKER_DIR}")
set(LINKER_SCRIPT "${LINKER_DIR}/STM32G474RE_FLASH.ld")

if(NOT EXISTS "${LINKER_SCRIPT}")
    file(WRITE "${LINKER_SCRIPT}" "ENTRY(Reset_Handler)

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Top of stack = end of RAM (stack grows downward) */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } >RAM

  ._user_heap_stack :
  {
    . = ALIGN(8);
    . = . + 0x200; /* min heap headroom */
    . = . + 0x400; /* min stack headroom */
    . = ALIGN(8);
  } >RAM
}
")
    message(STATUS "Wrote STM32G474RE_FLASH.ld")
else()
    message(STATUS "STM32G474RE_FLASH.ld already present, skipping")
endif()

message(STATUS "")
message(STATUS "== Environment ready ==")
message(STATUS "  make list                    -- see buildable projects")
message(STATUS "  make <project_name>          -- build one")
message(STATUS "  make flash-<project_name>    -- flash it")
