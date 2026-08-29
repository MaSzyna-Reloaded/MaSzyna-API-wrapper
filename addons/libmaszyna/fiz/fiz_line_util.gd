@tool
extends RefCounted
class_name FizLineUtil

## Shared helpers built on top of a MaszynaParser instance already scoped to a single FIZ
## line (see FizTrainControllerInstancer, which creates one such instance per line and hands
## it to the matching section parser's parse()/parse_row(), exactly like the original
## LoadFIZ_* code constructing "a fresh cParser parser(line)" per line/row). Comment stripping
## (`//`, `/* */`) is MaszynaParser's own job during tokenization - nothing here reimplements
## it.

const _STOPS: Array = [" ", "\t", "\n", "\r", ";"]


## Pulls every `Key=Value` token out of `p` (until eof) into a String->String dictionary. Keys
## are kept case-sensitive, matching the original `extract_value`'s exact-substring lookup.
static func read_key_values(p: MaszynaParser) -> Dictionary:
    var result: Dictionary = {}
    while not p.eof_reached():
        var token: String = p.next_token(_STOPS)
        if token.is_empty():
            continue
        var eq: int = token.find("=")
        if eq <= 0:
            continue
        result[token.substr(0, eq)] = token.substr(eq + 1)
    return result


static func get_string(kv: Dictionary, key: String, default_value: String = "") -> String:
    return kv.get(key, default_value)


static func has_key(kv: Dictionary, key: String) -> bool:
    return kv.has(key)


static func get_float(kv: Dictionary, key: String, default_value: float = 0.0) -> float:
    var v: String = kv.get(key, "")
    if v.is_empty():
        return default_value
    return v.to_float()


static func get_int(kv: Dictionary, key: String, default_value: int = 0) -> int:
    var v: String = kv.get(key, "")
    if v.is_empty():
        return default_value
    return v.to_int()


## FIZ boolean convention: literal `Yes`/`No` tokens (case-insensitive). Anything else -
## including an absent key - is false.
static func get_bool(kv: Dictionary, key: String, default_value: bool = false) -> bool:
    var v: String = kv.get(key, "")
    if v.is_empty():
        return default_value
    return v.to_lower() == "yes"
