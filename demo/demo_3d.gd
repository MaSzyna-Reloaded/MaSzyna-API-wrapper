extends Node3D


## Before _ready(): the vehicles under this node must not read a cache left by another build
func _enter_tree() -> void:
    SimulationServer.check_build_version()


func _ready() -> void:
    TrackServer.topology_rebuild()
