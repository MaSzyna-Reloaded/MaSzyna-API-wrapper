@tool
extends ScenarioEventAction
class_name MaszynaLegacySoundAction

## The original's `sound` event (sound_event, Event.cpp:1394-1429): plays, loops or stops its
## scenery sounds. Played with a radio channel it is a radio message instead, heard only on the
## player's cab radio (simulation::radio_message(), CabinSystem.send_radio_message()).

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
## Each player's range [m] (the scenery node's rmax): a radio message reaches only as far
var reaches:PackedFloat64Array = []
@export var mode:Mode = Mode.PLAY
## The radio channel the sound is a message on, 0 for none
@export var radio_channel:int = 0


func _run(_event:RID, _activator:RID) -> void:
    for index:int in players.size():
        var player:SfxPlayer3D = players[index]
        if not is_instance_valid(player):
            continue
        if mode == Mode.PLAY and radio_channel > 0:
            CabinSystem.send_radio_message(
                    player.bank.get_event(PLAY_EVENT), radio_channel, player.global_position, reaches[index])
            continue
        if mode == Mode.STOP:
            player.stop()
            continue
        # exclusive, as the original plays it: a sound already playing is left as it is
        # (sound_flags::exclusive, sound_source::play_basic(), sound.cpp:403-421)
        if player.is_playing(PLAY_EVENT) or player.is_playing(LOOP_EVENT):
            continue
        player.play(PLAY_EVENT if mode == Mode.PLAY else LOOP_EVENT)
