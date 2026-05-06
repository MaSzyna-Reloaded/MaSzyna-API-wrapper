@tool
extends Object

func import(p:MaszynaParser, context: MaszynaImporterContext):
    context.pop_origin()
    return []
