#!/bin/sh
set -eu

tool_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
source_file="$tool_dir/src/hide_guard_linux.c"
binary=${1-}
mode=${2-nowrite}

forbidden='XMapWindow|XUnmapWindow|XRaiseWindow|XLowerWindow|XStoreName|XDeleteProperty|XSetWMName|XSetTextProperty|XChangeWindowAttributes|XConfigureWindow|XMoveWindow|XResizeWindow|XMoveResizeWindow|XReparentWindow|XDestroyWindow|XCreateWindow|XSendEvent|XSetInputFocus|XGrab|XUngrab|XKillClient|/dev/fb|HWTCON|MXCFB|lipc-set|CLOCK_BOOTTIME_ALARM|ioctl[[:space:]]*\('

if grep -En "$forbidden" "$tool_dir"/src/*.c "$tool_dir"/include/*.h; then
    echo "audit: forbidden source capability found" >&2
    exit 1
fi

change_calls=$(grep -Ec '^[[:space:]]*XChangeProperty[[:space:]]*\(' "$source_file")
if [ "$change_calls" -ne 1 ]; then
    echo "audit: expected exactly one XChangeProperty call site" >&2
    exit 1
fi

if [ -n "$binary" ]; then
    if [ ! -f "$binary" ]; then
        echo "audit: binary not found: $binary" >&2
        exit 1
    fi
    if nm -u "$binary" | grep -E 'XMapWindow|XUnmapWindow|XRaiseWindow|XLowerWindow|XStoreName|XDeleteProperty|XSetWMName|XSetTextProperty|XChangeWindowAttributes|XConfigureWindow|XMoveWindow|XResizeWindow|XMoveResizeWindow|XReparentWindow|XDestroyWindow|XCreateWindow|XSendEvent|XSetInputFocus|XGrab|XUngrab|XKillClient'; then
        echo "audit: forbidden X11 import found" >&2
        exit 1
    fi
    case "$mode" in
        nowrite)
            if nm -u "$binary" | grep -q 'XChangeProperty'; then
                echo "audit: write-disabled binary imports XChangeProperty" >&2
                exit 1
            fi
            ;;
        write)
            if ! nm -u "$binary" | grep -q 'XChangeProperty'; then
                echo "audit: write-enabled binary lacks reviewed writer" >&2
                exit 1
            fi
            ;;
        *)
            echo "audit: mode must be nowrite or write" >&2
            exit 1
            ;;
    esac
fi

echo "audit: PASS mode=$mode"
