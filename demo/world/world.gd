class_name SceneryWorld
extends Node3D

## The 3D world of a scenery - its environment, the player, the scenery itself, its weather sounds
## and its keys. The menu has none of it: the world is made when a scenery is chosen and freed when
## the player goes back to the menu (demo_scenery_loading.gd), so the menu renders nothing behind it.

## A step of the load (MaszynaIncludeNode.load_progress)
signal load_progress(progress: float, stage: MaszynaIncludeNode.LoadStage, message: String)
## The includes parsed so far (MaszynaIncludeNode.load_files_parsed)
signal load_files_parsed(count: int, filename: String)
## The scenery has loaded, with its first vehicle's train_id (MaszynaSceneryNode.scenery_loaded)
signal scenery_loaded(first_train_id: String)
## The share of the scenario's start done (0..1), between the load and scenery_loaded
## (MaszynaLegacyScenario.start())
signal scenario_progress(progress: float)

## The scenery's scenario, running from its load until it unloads
var _scenario: MaszynaLegacyScenario = null


## Loads scenery/<filename>, the player's trainset as arranged (MaszynaIncludeNode.trainset_override),
## and starts its scenario - done once `scenery_loaded` has been emitted
func load_scenery(filename: String, trainset: Array[MaszynaDynamicData], train_id: String = "") -> void:
    %MaszynaSceneryNode.filename = filename
    %MaszynaSceneryNode.trainset_override.assign(trainset)
    %Player.clear_start_train()
    %Player.start_vehicle_id = train_id
    %MaszynaSceneryNode.load()
    await %Player.initialization_ready


## Frees what the scenery holds, spread over frames - the world itself goes with queue_free()
func unload_scenery() -> void:
    %Player.clear_start_train()
    %MaszynaSceneryNode.filename = ""
    await %MaszynaSceneryNode.load()


## The ScenarioScriptServer context the scenery's scripts run in, invalid without a scenario
func get_script_context() -> RID:
    return _scenario.get_script_context() if _scenario else RID()


func get_environment() -> MaszynaEnvironmentNode:
    return %MaszynaEnvironmentNode


## The scenery node: its file, title and start time
func get_scenery() -> MaszynaSceneryNode:
    return %MaszynaSceneryNode


func _on_scenery_load_progress(progress: float, stage: MaszynaIncludeNode.LoadStage, message: String) -> void:
    load_progress.emit(progress, stage, message)


func _on_scenery_load_files_parsed(count: int, filename: String) -> void:
    load_files_parsed.emit(count, filename)


## The game runs the scenario of a loaded scenery; the scenery only loads it
func _on_scenery_loaded(first_train_id: String) -> void:
    _scenario = MaszynaLegacyScenario.new()
    # started - a scenery without sounds tells no share of it on the way
    scenario_progress.emit(0.0)
    await _scenario.start(%MaszynaSceneryNode, scenario_progress.emit)
    if not %Player.start_vehicle_id:
        %Player.start_vehicle_id = first_train_id
    scenery_loaded.emit(first_train_id)


func _on_scenery_unloading() -> void:
    if _scenario:
        _scenario.stop()
        _scenario = null
