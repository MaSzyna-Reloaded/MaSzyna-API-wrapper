@tool
extends RefCounted

## Original engine's "atmo:" token (deserialize_atmo(), simulationstateserializer.cpp) carries a
## randomized fog-distance range and an optional overcast value that feeds weather computation -
## no defined mapping onto MaszynaEnvironmentNode's cloudiness/weather exists yet, so this only
## consumes the tokens to keep the parser stream in sync. Left unimplemented until that mapping
## is worked out.
func import(p: MaszynaParser, _context: MaszynaImporterContext) -> Array:
    p.get_tokens_until("endatmo")
    return []
