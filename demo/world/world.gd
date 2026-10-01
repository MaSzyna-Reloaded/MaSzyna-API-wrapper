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


## Loads scenery/<filename>, its vehicles with the skins overridden
func load_scenery(filename: String, skin_overrides: Dictionary) -> void:
    %MaszynaSceneryNode.filename = filename
    %MaszynaSceneryNode.skin_overrides.assign(skin_overrides)
    %Player.clear_start_train()
    await %MaszynaSceneryNode.load()


## Frees what the scenery holds, spread over frames - the world itself goes with queue_free()
func unload_scenery() -> void:
    %Player.clear_start_train()
    %MaszynaSceneryNode.filename = ""
    await %MaszynaSceneryNode.load()


## The vehicle the player takes once the scenery is loaded
func start_player(train_id: String) -> void:
    %Player.start_vehicle_id = train_id


## The ScenarioScriptServer context the scenery's scripts run in
func get_script_context() -> RID:
    return %MaszynaSceneryNode.get_script_context()


func get_environment() -> MaszynaEnvironmentNode:
    return %MaszynaEnvironmentNode


func _on_scenery_load_progress(progress: float, stage: MaszynaIncludeNode.LoadStage, message: String) -> void:
    load_progress.emit(progress, stage, message)


func _on_scenery_load_files_parsed(count: int, filename: String) -> void:
    load_files_parsed.emit(count, filename)


func _on_scenery_loaded(first_train_id: String) -> void:
    scenery_loaded.emit(first_train_id)
