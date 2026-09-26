@tool
extends ScenarioEventAction
class_name MaszynaLegacySoundAction

## The original's `sound` event (sound_event, Event.cpp:1394-1429): plays, loops or stops its
## scenery sounds. The radio message a channel makes of it is not ported (TODO.md).

enum Mode {
    ## 0
    STOP,
    ## 1
    PLAY,
    ## -1
    LOOP,
}

## The events of a scenery sound's bank (MaszynaLegacyEventFactory)
const PLAY_EVENT:StringName = &"play"
const LOOP_EVENT:StringName = &"loop"

var players:Array[SfxPlayer3D] = []
@export var mode:Mode = Mode.PLAY


func _run(_event:RID, _activator:RID) -> void:
    for player:SfxPlayer3D in players:
        if not is_instance_valid(player):
            continue
        if mode == Mode.STOP:
            player.stop()
            continue
        # exclusive, as the original plays it: a sound already playing is left as it is
        # (sound_flags::exclusive, sound_source::play_basic(), sound.cpp:403-421)
        if player.is_playing(PLAY_EVENT) or player.is_playing(LOOP_EVENT):
            continue
        player.play(PLAY_EVENT if mode == Mode.PLAY else LOOP_EVENT)
