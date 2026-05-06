#!/bin/bash
set -euo pipefail

cd /var/maszyna || exit 1

target_file="src/register_types.cpp"
backup_file="$(mktemp)"
log_file="/var/maszyna/ci/hotreload-test.log"
build_dir="build-hotreload-test"
editor_pid=""

cp "$target_file" "$backup_file"
rm -f "$log_file"

cleanup() {
    cp "$backup_file" "$target_file"
    rm -f "$backup_file"

    if [ -n "$editor_pid" ] && kill -0 "$editor_pid" 2>/dev/null; then
        kill "$editor_pid" || true
        wait "$editor_pid" || true
    fi
}

trap cleanup EXIT

echo "Building initial extension..."
cmake -B "$build_dir" -DGODOTCPP_TARGET=template_debug
cmake --build "$build_dir" --parallel

echo "Starting Godot editor..."
godot --editor --headless --path demo >"$log_file" 2>&1 &
editor_pid=$!

echo "Waiting for editor startup..."
timeout 120 bash -lc 'until grep -q "Initializing libmaszyna module on level 3.0" "$1"; do sleep 1; done' _ "$log_file"

echo "Applying hot reload marker patch..."
python3 - <<'PY'
from pathlib import Path

path = Path("/var/maszyna/src/register_types.cpp")
text = path.read_text()
needle = '    UtilityFunctions::print("Initializing libmaszyna module on level " + String::num(p_level) + "...");\n'
replacement = needle + '    UtilityFunctions::print("HOTRELOAD");\n'

if "UtilityFunctions::print(\"HOTRELOAD\");" in text:
    raise SystemExit("HOTRELOAD marker already present in src/register_types.cpp")

if needle not in text:
    raise SystemExit("Could not find initialization log line in src/register_types.cpp")

path.write_text(text.replace(needle, replacement, 1))
PY

echo "Rebuilding extension to trigger hot reload..."
cmake --build "$build_dir" --parallel

echo "Waiting for hot reload..."
timeout 120 bash -lc 'until grep -q "HOTRELOAD" "$1"; do sleep 1; done' _ "$log_file"

if ! kill -0 "$editor_pid" 2>/dev/null; then
    cat "$log_file"
    echo "Godot editor crashed during hot reload."
    exit 1
fi

echo "Hot reload completed without crashing."
