#!/bin/sh
# Builds web/star.js (wasm embedded) for the playground. Needs Emscripten (em++) on PATH.
set -e
cd "$(dirname "$0")"
em++ -O2 -fexceptions -lembind ../src/star_wasm.cpp -o star.js \
    -sMODULARIZE=1 -sEXPORT_NAME=createStar -sSINGLE_FILE=1 -sENVIRONMENT=web,node
echo "Built web/star.js - open web/index.html in a browser"
