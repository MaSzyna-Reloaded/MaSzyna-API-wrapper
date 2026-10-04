class_name SettingResolution
extends SettingOption

## The window's size: the usual sizes smaller than the screen the game's window is on, and that
## screen's own as the largest. The value is the size; one no choice holds (the default - no size,
## a size stored on another screen) shows as the screen's own.

## The usual sizes, 16:9 and 16:10, smallest first
const SIZES: Array[Vector2i] = [
    Vector2i(1280, 720), Vector2i(1280, 800), Vector2i(1366, 768), Vector2i(1440, 900),
    Vector2i(1600, 900), Vector2i(1680, 1050), Vector2i(1920, 1080), Vector2i(1920, 1200),
    Vector2i(2560, 1440), Vector2i(2560, 1600), Vector2i(3200, 1800), Vector2i(3840, 2160),
]

## The choices' sizes, by id
var _sizes: Array[Vector2i] = []


func _setup() -> void:
    # the game's own window's screen - the settings stand in a window of their own, embedded in it
    var screen: Vector2i = DisplayServer.screen_get_size(get_tree().root.current_screen)
    for size: Vector2i in SIZES:
        if size.x < screen.x and size.y < screen.y:
            _sizes.append(size)
    _sizes.append(screen)
    for id: int in _sizes.size():
        %Options.add_item("%d × %d" % [_sizes[id].x, _sizes[id].y], id)


func _value_of(id: int) -> Variant:
    return _sizes[id]


func _id_of(value: Variant) -> int:
    var id: int = _sizes.find(value) if value is Vector2i else -1
    return id if id >= 0 else _sizes.size() - 1
