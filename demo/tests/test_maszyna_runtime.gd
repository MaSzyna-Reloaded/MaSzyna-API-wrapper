extends MaszynaGutTest

## The cache directory is the test's own, so nothing the game caches is touched
const CACHE_DIRECTORY:String = "test_maszyna_runtime"

var _cache:ResourceCache


func before_each():
    _cache = ResourceCache.create(CACHE_DIRECTORY)

func after_each():
    _cache.clear()
    _cache = null

func test_clear_cache_emits_the_request():
    watch_signals(MaszynaRuntime)
    MaszynaRuntime.clear_cache()
    assert_signal_emitted(MaszynaRuntime, "cache_clear_requested")

func test_a_created_cache_follows_the_request():
    var resource:Resource = Resource.new()
    _cache.set("entry.res", resource)
    assert_true(_cache.has("entry.res"), "the entry should be in the cache before it is cleared")

    MaszynaRuntime.clear_cache()
    assert_false(_cache.has("entry.res"), "clear_cache() should drop the entry of every cache")

func test_build_number_is_the_stamp_of_the_build():
    var stamp:String = MaszynaRuntime.get_build_number()
    assert_eq(stamp, FileAccess.get_file_as_string("res://build_number.txt").strip_edges())
