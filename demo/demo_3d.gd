extends Node3D


## Before _ready(): the vehicles under this node must not read a cache left by another build
func _enter_tree() -> void:
    MaszynaRuntime.check_build_version()


func _ready() -> void:
    TrackManager.topology_rebuild()
