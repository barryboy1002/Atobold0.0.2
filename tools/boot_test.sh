#!/bin/sh
# Headless boot test for Atobold.
#
# usage: tools/boot_test.sh [--panic] <image>
#
# Boots the image in QEMU with no display, gives the kernel a few
# seconds to run, then checks:
#   normal mode:  the boot banner on COM1 and in VGA text memory
#   --panic mode: the exception report on COM1 (see interrupts.c)
#
# QEMU can be overridden via $QEMU (default qemu-system-i386).
set -u

PANIC=0
if [ "${1:-}" = "--panic" ]; then
  PANIC=1
  shift
fi
IMAGE=${1:?usage: tools/boot_test.sh [--panic] <image>}
QEMU=${QEMU:-qemu-system-i386}
SERIAL_LOG="$(dirname "$IMAGE")/serial.log"
MONITOR_LOG="$(dirname "$IMAGE")/monitor.log"

rm -f "$SERIAL_LOG" "$MONITOR_LOG"

# boot headless; after a few seconds dump the first VGA text cells and quit
( sleep 4; echo "xp /2hx 0xb8000"; sleep 1; echo quit ) | \
  timeout 30 $QEMU -drive format=raw,file="$IMAGE" -display none \
    -no-reboot -serial file:"$SERIAL_LOG" -monitor stdio \
    > "$MONITOR_LOG" 2>&1

if [ ! -s "$SERIAL_LOG" ]; then
  echo "FAIL: no serial output at all (is the serial driver initialized?)"
  exit 1
fi

if [ "$PANIC" = 1 ]; then
  if command grep -q "KERNEL PANIC" "$SERIAL_LOG" && \
     command grep -q "EXCEPTION" "$SERIAL_LOG"; then
    echo "PASS: kernel panicked as expected and reported the exception"
    exit 0
  fi
  echo "FAIL: expected a kernel panic report on serial, got:"
  tail -n +1 "$SERIAL_LOG"
  exit 1
fi

if ! command grep -q "Atobold 0.0.2" "$SERIAL_LOG"; then
  echo "FAIL: boot banner missing from serial output:"
  tail -n +1 "$SERIAL_LOG"
  exit 1
fi

# first VGA cell should be the 'A' of the banner (0x41) with the
# white-on-grey attribute (0x8F): halfword 0x8f41
if ! command grep -qi "0x8f41" "$MONITOR_LOG"; then
  echo "FAIL: banner not found in VGA text memory; monitor output:"
  tail -n +1 "$MONITOR_LOG"
  exit 1
fi

echo "PASS: kernel booted; banner on COM1 and on screen"
