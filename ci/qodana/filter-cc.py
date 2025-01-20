#!/usr/bin/env python3
import json
from pathlib import Path

rootdir = Path.cwd()
compile_commands_path = rootdir / "build/compile_commands.json"
include_glob = "src/**/*"

with open(compile_commands_path, "r", encoding="utf-8") as fd:
    cc = json.load(fd)

# keep only sources from src/ in compile_commands
allowed_paths = set(rootdir.glob(include_glob))


def keep_condition(cc_entry: dict):
    path = Path(cc_entry["file"])
    return path in allowed_paths


cc = list(filter(keep_condition, cc))
with open(compile_commands_path, "w", encoding="utf-8") as fd:
    json.dump(cc, fd, ensure_ascii=False, indent="\t")