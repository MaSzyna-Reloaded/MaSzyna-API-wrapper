extends MaszynaGutTest

## ResourceLazyLoader: a resource is loaded when it is wanted, shared by its key, held while it is
## fetched and let go once nobody holds it.

var _loads:int = 0
var _rids:Array[RID] = []


func before_each() -> void:
    _loads = 0


func after_each() -> void:
    for rid:RID in _rids:
        ResourceLazyLoader.resource_free(rid)
    _rids.clear()


func test_one_key_is_one_resource() -> void:
    var first:RID = _register("test/shared")
    var second:RID = _register("test/shared")
    assert_eq(first, second, "one key gave two resources")
    assert_same(ResourceLazyLoader.resource_fetch(first), ResourceLazyLoader.resource_fetch(second))
    assert_eq(_loads, 1, "a shared resource was loaded twice")
    ResourceLazyLoader.resource_release(first)
    ResourceLazyLoader.resource_release(second)


func test_resource_is_held_until_the_last_release() -> void:
    var rid:RID = _register("test/held")
    assert_false(ResourceLazyLoader.resource_is_resident(rid), "resident before anybody fetched it")
    ResourceLazyLoader.resource_fetch(rid)
    ResourceLazyLoader.resource_fetch(rid)
    ResourceLazyLoader.resource_release(rid)
    assert_true(ResourceLazyLoader.resource_is_resident(rid), "let go while still held")
    ResourceLazyLoader.resource_release(rid)
    assert_false(ResourceLazyLoader.resource_is_resident(rid), "held after the last release")


func test_loaded_copy_still_alive_is_not_loaded_again() -> void:
    var rid:RID = _register("test/alive")
    var loaded:Resource = ResourceLazyLoader.resource_load(rid)
    assert_false(ResourceLazyLoader.resource_is_resident(rid), "a load alone holds the resource")
    assert_same(ResourceLazyLoader.resource_fetch(rid), loaded, "the live copy was not handed out")
    assert_eq(_loads, 1, "loaded again while a copy was alive")
    ResourceLazyLoader.resource_release(rid)


func test_released_resource_is_loaded_again() -> void:
    var rid:RID = _register("test/reloaded")
    ResourceLazyLoader.resource_fetch(rid)
    ResourceLazyLoader.resource_release(rid)
    ResourceLazyLoader.resource_fetch(rid)
    assert_eq(_loads, 2, "a released resource was kept")
    ResourceLazyLoader.resource_release(rid)


func test_data_reload_lets_go_of_held_resources() -> void:
    var rid:RID = _register("test/unloaded")
    var before:Resource = ResourceLazyLoader.resource_fetch(rid)
    GameDataServer.data_reload()
    assert_false(ResourceLazyLoader.resource_is_resident(rid), "held over the data reload")
    assert_not_same(ResourceLazyLoader.resource_fetch(rid), before, "the old data was handed out again")
    ResourceLazyLoader.resource_release(rid)
    ResourceLazyLoader.resource_release(rid)


func _register(key:String) -> RID:
    var rid:RID = ResourceLazyLoader.resource_register(key, _load)
    _rids.append(rid)
    return rid


func _load() -> Resource:
    _loads += 1
    return Resource.new()
