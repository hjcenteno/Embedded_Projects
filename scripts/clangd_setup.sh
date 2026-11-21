#!/usr/bin/env bash
# Generates a machine-specific .clangd. Override the CMSIS paths with env vars if yours differ:
#   CMSIS_CORE=/path/to/cmsis_core CMSIS_DEVICE=/path/to/cmsis_device_g4 ./scripts/setup_clangd.sh
cd "$(dirname "$0")/.." || exit 1
ROOT=$PWD
CMSIS_CORE=${CMSIS_CORE:-$HOME/cmsis_core}
CMSIS_DEVICE=${CMSIS_DEVICE:-$HOME/cmsis_device_g4}

cat > .clangd <<EOF
CompileFlags:
  Add:
    - --target=arm-none-eabi
    - -mcpu=cortex-m4
    - -mthumb
    - -mfpu=fpv4-sp-d16
    - -mfloat-abi=hard
    - -std=gnu23
    - -DSTM32G474xx
    - -I$CMSIS_CORE/CMSIS/Core/Include
    - -I$CMSIS_DEVICE/Include
    - -I$ROOT/lib
    - -isystem
    - /usr/lib/arm-none-eabi/include
Diagnostics:
  UnusedIncludes: None
EOF
echo "Wrote $ROOT/.clangd"
EOF