#!/usr/bin/env bash
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

echo "→ Cloning upstream ImStudio..."
git clone --depth 1 https://github.com/Raais/ImStudio "$TMP/ImStudio"

VD="$ROOT/android/app/src/main/cpp/vendor"
mkdir -p "$VD"/{imstudio,imgui,fmt}

cp -f "$TMP"/ImStudio/src/sources/ims_generator.cpp "$VD/imstudio/"
cp -f "$TMP"/ImStudio/src/sources/ims_generator.h   "$VD/imstudio/"
cp -f "$TMP"/ImStudio/src/sources/ims_buffer.cpp    "$VD/imstudio/"
cp -f "$TMP"/ImStudio/src/sources/ims_buffer.h      "$VD/imstudio/"
cp -f "$TMP"/ImStudio/src/sources/ims_object.h      "$VD/imstudio/"
[ -f "$TMP"/ImStudio/src/utils/ims_utils.cpp ] && cp -f "$TMP"/ImStudio/src/utils/ims_utils.cpp "$VD/imstudio/" || true
[ -f "$TMP"/ImStudio/src/utils/ims_utils.h ]   && cp -f "$TMP"/ImStudio/src/utils/ims_utils.h   "$VD/imstudio/" || true

cp -f "$TMP"/ImStudio/src/third-party/imgui/imgui*.cpp "$VD/imgui/" 2>/dev/null || true
cp -f "$TMP"/ImStudio/src/third-party/imgui/imgui*.h   "$VD/imgui/" 2>/dev/null || true
cp -f "$TMP"/ImStudio/src/third-party/imgui/imconfig.h "$VD/imgui/" 2>/dev/null || true
cp -f "$TMP"/ImStudio/src/third-party/imgui/imstb_*.h  "$VD/imgui/" 2>/dev/null || true

if [ -d "$TMP"/ImStudio/src/third-party/fmt/include/fmt ]; then
    mkdir -p "$VD/fmt/include/fmt"
    cp -f "$TMP"/ImStudio/src/third-party/fmt/include/fmt/*.h "$VD/fmt/include/fmt/"
fi

SHA=$(cd "$TMP/ImStudio" && git rev-parse --short HEAD)
echo "$SHA" > "$ROOT/.upstream-commit"
echo "✓ Synced upstream commit $SHA"
