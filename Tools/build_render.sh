#!/usr/bin/env bash
# usage: Tools/build_render.sh legacy|new OUT_BINARY [extra g++ flags]
set -euo pipefail
cd "$(dirname "$0")/.."
which=${1:?legacy|new}; out=${2:?output binary}; shift 2 || true
case "$which" in
  legacy) hdr='"legacy/DrumEngine_legacy.h"' ;;
  new)    hdr='"../Source/DrumEngine.h"' ;;
  *) echo "unknown engine $which"; exit 1 ;;
esac
g++ -O2 -std=c++17 -Wall -Wextra -DENGINE_HEADER="$hdr" Tools/render_demo.cpp -o "$out" "$@" && echo "OK -> $out"
