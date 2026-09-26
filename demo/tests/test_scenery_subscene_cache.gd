extends MaszynaGutTest

## Large parameterless includes are parsed as subscenes cached per inherited origin/rotate; a cached
## subscene gives the same result as parsing it in place.

const FIXTURES_GAME_DIR:String = "res://tests/fixtures"
const ROOT:String = "subscene/root.scn"

var _previous_game_dir:String = ""
var _cache_dir:String = "user://cache".path_join(SceneryInstancer.CACHE_DIRECTORY)
var _existing_files:PackedStringArray = []


func before_each() -> void:
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    UserSettings.save_maszyna_game_dir(FIXTURES_GAME_DIR)
    _existing_files = _list_cache_files()


func after_each() -> void:
    for file:String in _new_cache_files():
        DirAccess.remove_absolute(_cache_dir.path_join(file))
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func test_subscenes_are_cached_per_origin_and_match_in_place_parsing() -> void:
    var in_place := MaszynaImporterContext.new()
    SceneryInstancer.parse_file(ROOT, {}, in_place)

    var parsed:MaszynaImporterContext = _parse_threaded()
    # big.scm twice without parameters (two origins); with parameters and small.inc are not cached
    assert_eq(_new_cache_files().size(), 2)
    var cached:MaszynaImporterContext = _parse_threaded()
    assert_eq(_new_cache_files().size(), 2)

    var expected:Array = _describe(in_place)
    assert_eq(expected[0].size(), 7)
    assert_eq(expected[1].size(), 3)
    assert_eq(_describe(parsed), expected)
    assert_eq(_describe(cached), expected)
    assert_eq(cached.dependencies.keys().size(), 3)
    assert_true(cached.cacheable)


func _parse_threaded() -> MaszynaImporterContext:
    var queue := SceneryLoadingTaskQueue.new()
    return SceneryInstancer.parse_file_task(ROOT, {}, MaszynaImporterContext.new().get_state(), queue)


## [models as [filename, position], triangle entries]
func _describe(context:MaszynaImporterContext) -> Array:
    var models:Array = context.models.map(
        func(model:MaszynaModelData) -> Array: return [model.model_filename, model.position]
    )
    return [models, context.triangles]


func _new_cache_files() -> PackedStringArray:
    var files:PackedStringArray = []
    for file:String in _list_cache_files():
        if not _existing_files.has(file):
            files.append(file)
    return files


func _list_cache_files() -> PackedStringArray:
    return DirAccess.get_files_at(_cache_dir)
