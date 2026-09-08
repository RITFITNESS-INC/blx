#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
PROJECT_DIR="$ROOT_DIR/tension_knob_tr2"
BUILD_DIR="$ROOT_DIR/.build/tr2"
FIRMWARE_DIR="$ROOT_DIR/firmware"
TOOLCHAIN_DIR=${TR2_TOOLCHAIN:-/tmp/xpack-riscv-none-embed-gcc-10.2.0-1.2}
CC="$TOOLCHAIN_DIR/bin/riscv-none-embed-gcc"
STRIP="$TOOLCHAIN_DIR/bin/riscv-none-embed-strip"
NM="$TOOLCHAIN_DIR/bin/riscv-none-embed-nm"
TEMPLATE="$PROJECT_DIR/libs/platform/tr2grfdata-template.bin"

if [ ! -x "$CC" ] || [ ! -x "$STRIP" ] || [ ! -x "$NM" ]; then
    echo "Missing xPack RISC-V GCC 10.2 toolchain at: $TOOLCHAIN_DIR" >&2
    echo "Set TR2_TOOLCHAIN to the extracted xPack toolchain directory." >&2
    exit 1
fi
if ! command -v mcopy >/dev/null 2>&1 || ! command -v mdel >/dev/null 2>&1; then
    echo "mtools is required (macOS: brew install mtools)." >&2
    exit 1
fi
if [ ! -f "$TEMPLATE" ]; then
    echo "Missing official TR2 data-image template: $TEMPLATE" >&2
    exit 1
fi

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR/obj" "$FIRMWARE_DIR/grf_update"

find "$PROJECT_DIR/apps" -name '*.c' -print | while IFS= read -r source; do
    relative=${source#"$PROJECT_DIR/"}
    object_name=$(printf '%s' "$relative" | tr '/' '_')
    arch -x86_64 "$CC" \
        -std=gnu11 -Os -fPIC -ffunction-sections -fdata-sections \
        -march=rv32imafc -mabi=ilp32f -Wall -Wextra -Werror \
        -I"$PROJECT_DIR/inc" -I"$PROJECT_DIR/libs/appscc" \
        -c "$source" -o "$BUILD_DIR/obj/$object_name.o"
done

arch -x86_64 "$CC" \
    -shared -nostdlib -march=rv32imafc -mabi=ilp32f \
    -Wl,-e,main -Wl,--gc-sections \
    -o "$BUILD_DIR/tr2grfui.mo" \
    "$BUILD_DIR"/obj/*.o "$PROJECT_DIR/libs/platform/libs_a/libtr2grfui.a"
arch -x86_64 "$STRIP" --strip-unneeded "$BUILD_DIR/tr2grfui.mo"
unsupported_helpers=$(arch -x86_64 "$NM" -D -u "$BUILD_DIR/tr2grfui.mo" |
    awk '/__(divdi3|moddi3|udivdi3|umoddi3)$/ { print }')
if [ -n "$unsupported_helpers" ]; then
    echo "TR2 module uses unsupported 64-bit arithmetic helpers:" >&2
    echo "$unsupported_helpers" >&2
    exit 1
fi
cp "$PROJECT_DIR/res/config/project/grfprj.g" "$BUILD_DIR/grfprj.g"
cp "$PROJECT_DIR/res/config/view/tension.g" "$BUILD_DIR/tension.g"
cp "$PROJECT_DIR/res/font/HarmonyOS_Sans_SC_Regular.ttf" "$BUILD_DIR/HarmonyOS_Sans_SC_Regular.ttf"
touch -t 202609010000 \
    "$BUILD_DIR/tr2grfui.mo" "$BUILD_DIR/grfprj.g" "$BUILD_DIR/tension.g" \
    "$BUILD_DIR/HarmonyOS_Sans_SC_Regular.ttf"

cp "$TEMPLATE" "$BUILD_DIR/tr2grfdata.bin"
mdel -i "$BUILD_DIR/tr2grfdata.bin" ::/GRF_APP/RES/CONFIG/VIEW/PROCESSING.G
mdel -i "$BUILD_DIR/tr2grfdata.bin" ::/GRF_APP/RES/CONFIG/VIEW/TEMP.G
mdel -i "$BUILD_DIR/tr2grfdata.bin" ::/GRF_APP/RES/CONFIG/VIEW/VIEW1.G
mdel -i "$BUILD_DIR/tr2grfdata.bin" ::/GRF_APP/RES/CONFIG/VIEW/VOLUME.G
mcopy -om -i "$BUILD_DIR/tr2grfdata.bin" \
    "$BUILD_DIR/tr2grfui.mo" ::/GRF_APP/TR2GRFUI.MO
mcopy -om -i "$BUILD_DIR/tr2grfdata.bin" \
    "$BUILD_DIR/grfprj.g" \
    ::/GRF_APP/RES/CONFIG/PROJECT/GRFPRJ.G
mcopy -om -i "$BUILD_DIR/tr2grfdata.bin" \
    "$BUILD_DIR/tension.g" \
    ::/GRF_APP/RES/CONFIG/VIEW/TENSION.G
mcopy -om -i "$BUILD_DIR/tr2grfdata.bin" \
    "$BUILD_DIR/HarmonyOS_Sans_SC_Regular.ttf" \
    ::/GRF_APP/RES/FONT/HARMONYOS_SANS_SC_REGULAR.TTF

cp "$BUILD_DIR/tr2grfui.mo" "$FIRMWARE_DIR/tr2grfui.mo"
cp "$BUILD_DIR/tr2grfdata.bin" "$FIRMWARE_DIR/grf_update/tr2grfdata.bin"
cp "$BUILD_DIR/tr2grfui.mo" "$PROJECT_DIR/libs/platform/tr2grfui.mo"
cp "$BUILD_DIR/tr2grfdata.bin" "$PROJECT_DIR/libs/platform/tr2grfdata.bin"

shasum -a 256 "$FIRMWARE_DIR/grf_update/tr2grfdata.bin" "$FIRMWARE_DIR/tr2grfui.mo"
