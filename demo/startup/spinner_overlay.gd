extends CanvasLayer

## EU07 spinner over the whole screen: shown while the main scene loads and while a scenery is
## unloaded ("Exit to menu"), so the stalls of those happen behind it.


func fade_in(duration: float) -> void:
    $Screen.modulate.a = 0.0
    visible = true
    var tween: Tween = create_tween()
    tween.tween_property($Screen, "modulate:a", 1.0, duration)
    await tween.finished


func fade_out(duration: float) -> void:
    var tween: Tween = create_tween()
    tween.tween_property($Screen, "modulate:a", 0.0, duration)
    await tween.finished
    visible = false


## Hides the loco and the track, keeping the background - what is below shows up only after fade_out()
func fade_out_spinner(duration: float) -> void:
    var tween: Tween = create_tween()
    tween.tween_property($Screen/Spinner, "modulate:a", 0.0, duration)
    await tween.finished
