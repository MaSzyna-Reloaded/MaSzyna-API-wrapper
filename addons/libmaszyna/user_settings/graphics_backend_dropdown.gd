@tool
extends OptionButton

const BACKEND_NAMES := {
    OS.RENDERING_DRIVER_VULKAN: "vulkan",
    OS.RENDERING_DRIVER_OPENGL3: "opengl3",
    OS.RENDERING_DRIVER_D3D12: "d3d12",
    OS.RENDERING_DRIVER_METAL: "metal",
}


func _ready():
    if OS.get_name() != "Windows":
        _remove_item_by_id(OS.RENDERING_DRIVER_D3D12)
    if OS.get_name() != "macOS":
        _remove_item_by_id(OS.RENDERING_DRIVER_METAL)

    var current_index := get_item_index(_detect_current_backend())
    if current_index != -1:
        select(current_index)

    item_selected.connect(_on_item_selected)


func _remove_item_by_id(id: int):
    var index := get_item_index(id)
    if index != -1:
        remove_item(index)


func _detect_current_backend() -> int:
    var backend_name := UserSettings.get_current_graphics_backend()
    for id in BACKEND_NAMES:
        if BACKEND_NAMES[id] == backend_name:
            return id
    return OS.RENDERING_DRIVER_VULKAN


func _on_item_selected(_index: int):
    UserSettings.save_graphics_backend(BACKEND_NAMES[get_selected_id()])
