extends GutHookScript

## The tests run as the game does: SimulationServer's clock ticks only with a SimulationRuntime in
## the tree (demo_scenery_loading.tscn places one), so the whole run gets one at its root


func run() -> void:
    gut.get_tree().root.add_child(SimulationRuntime.new())
