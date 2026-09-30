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

## The scenery sounds it plays (ScenerySoundServer)
var sounds:Array[RID] = []
## Each sound's range [m] (the scenery node's rmax): a radio message reaches only as far
var reaches:PackedFloat64Array = []
## Each sound's transcript, null for a sound with none, shown when the sound starts as heard
## (sound_source::update_counter(), sound.cpp:950-960)
var transcripts:Array[Transcript] = []
@export var mode:Mode = Mode.PLAY
## The radio channel the sound is a message on, 0 for none
@export var radio_channel:int = 0


func _run(_event:RID, _activator:RID) -> void:
    for index:int in sounds.size():
        var sound:RID = sounds[index]
        if mode == Mode.PLAY and radio_channel > 0:
            CabinSystem.send_radio_message(
                    ScenerySoundServer.sound_get_play_event(sound), transcripts[index], radio_channel,
                    ScenerySoundServer.sound_get_position(sound), reaches[index])
            continue
        if mode == Mode.STOP:
            ScenerySoundServer.sound_stop(sound)
            continue
        # a sound already playing goes on as it is, and shows no transcript again
        var started:bool = ScenerySoundServer.sound_play(
                sound, ScenerySoundServer.Playback.ONCE if mode == Mode.PLAY else ScenerySoundServer.Playback.LOOP)
        if started and transcripts[index]:
            TranscriptSystem.add(transcripts[index])
