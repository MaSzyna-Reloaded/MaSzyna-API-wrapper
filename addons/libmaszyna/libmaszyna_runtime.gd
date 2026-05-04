extends Node

const E3D_RESOURCE_FORMAT_LOADER: GDScript = preload("res://addons/libmaszyna/e3d/e3d_resource_format_loader.gd")
const OGG_VORBIS_FORMAT_LOADER: GDScript = preload("res://addons/libmaszyna/sound/ogg_vorbis_format_loader.gd")

var _e3d_loader: ResourceFormatLoader = null
var _ogg_vorbis_loader: ResourceFormatLoader = null


func _init() -> void:
    _e3d_loader = E3D_RESOURCE_FORMAT_LOADER.new()
    _ogg_vorbis_loader = OGG_VORBIS_FORMAT_LOADER.new()
    ResourceLoader.add_resource_format_loader(_e3d_loader)
    ResourceLoader.add_resource_format_loader(_ogg_vorbis_loader)


func _exit_tree() -> void:
    if _ogg_vorbis_loader:
        ResourceLoader.remove_resource_format_loader(_ogg_vorbis_loader)
        _ogg_vorbis_loader = null

    if _e3d_loader:
        ResourceLoader.remove_resource_format_loader(_e3d_loader)
        _e3d_loader = null
