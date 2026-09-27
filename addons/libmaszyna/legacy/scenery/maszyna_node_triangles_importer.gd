@tool
extends RefCounted

func import(p: MaszynaParser, context: MaszynaImporterContext, range_min: float, range_max: float):
    var triangle: Array = MaszynaTrianglesImporter.import_triangles(p, context.rotate, context.origin)
    if not triangle:
        return triangle
    triangle.append(range_min)
    triangle.append(range_max)
    return triangle
