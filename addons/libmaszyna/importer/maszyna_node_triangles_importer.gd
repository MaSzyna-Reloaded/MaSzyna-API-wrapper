@tool
extends RefCounted

func import(p: MaszynaParser, context: MaszynaImporterContext):
    return MaszynaTrianglesImporter.import_triangles(p, context.rotate, context.origin)
