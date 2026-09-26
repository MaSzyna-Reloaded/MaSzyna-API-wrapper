extends MaszynaGutTest

## Scenery "node model" becomes MaszynaModelData (built as an E3DRenderingServer RID by
## SceneryInstancer), not an E3DModelInstance node.

const NodeImporter = preload("res://addons/libmaszyna/importer/maszyna_node_importer.gd")


func test_model_node_is_imported_as_data_with_context_transform() -> void:
    var parser: MaszynaParser = MaszynaParser.new()
    parser.initialize("500 10 house model 1 2 3 90 budynki/dom.t3d skin1|skin2 endmodel".to_utf8_buffer())
    var context: MaszynaImporterContext = MaszynaImporterContext.new()
    context.origin = Vector3(100, 0, 0)

    var objects: Array = NodeImporter.new().import(parser, context)

    assert_false(objects.any(func(object: Variant) -> bool: return object is Node), "no nodes for models")
    assert_eq(context.models.size(), 1)
    var model: MaszynaModelData = context.models[0]
    assert_eq(model.data_path, "models/budynki")
    assert_eq(model.model_filename, "dom")
    assert_eq(model.skins, PackedStringArray(["skin1", "skin2"]))
    assert_eq(model.position, Vector3(101, 2, 3))
    assert_almost_eq(model.rotation.y, deg_to_rad(90.0), 0.0001)
    assert_eq(model.range_min, 10.0)
    assert_eq(model.range_max, 500.0)
