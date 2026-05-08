@tool
extends RefCounted


func import(p: MaszynaParser, _context: MaszynaImporterContext) -> Array:
    p.get_tokens_until("endatmo")
    return []
