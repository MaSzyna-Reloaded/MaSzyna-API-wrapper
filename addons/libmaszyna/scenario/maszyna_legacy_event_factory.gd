@tool
extends RefCounted
class_name MaszynaLegacyEventFactory

## Builds ScenarioEventServer's events, memories and launchers out of a scenery's data - the
## original's `firstinit` (simulationstateserializer.cpp:360-379). The original's encoding (the `*`
## of a field, `else`, a negative delay, HHMM) is read here once; the server gets delays, actions
## and conditions. Names are matched in lower case, as the original reads them (its parser lowers
## every token it is not told to keep, parser.h:71).
##
## Built: `updatevalues`, `addvalues`, `copyvalues`, `multiple`, `lights`, `switch`, `trackvel`,
## `voltage`, `animation` (rotate, translate); conditions `memcompare` and `probability`; a track's
## `event0/1/2`, `eventall0/1/2` and the events named `<track>:<slot>` (Track.cpp:970-983). The other
## types get an event without an action (TODO.md).

## "Leave this field as it is" (Event.cpp:491-503)
const FIELD_KEPT:String = "*"
## Splits a `multiple` event's list (Event.cpp:1266-1278)
const ELSE:String = "else"
## An event with this in its name is queued when the scenery starts (Event.cpp:2341-2345)
const ONSTART:String = "onstart"
## `copyvalues` copies all three fields unless given a mask (Event.cpp:898)
const ALL_FIELDS:int = ScenarioEventServer.MEMORY_FIELD_TEXT | ScenarioEventServer.MEMORY_FIELD_VALUE1 | ScenarioEventServer.MEMORY_FIELD_VALUE2
## `<text> <value1> <value2>`
const MEMCOMPARE_FIELDS:int = 3
## A launcher's HHMM (EvLaunch.cpp:139-140)
const HHMM_HOUR:int = 100
## A launcher firing the first time it is in range (EvLaunch.cpp:182-186) - not ported (TODO.md)
const FIRE_ONCE_IN_RANGE:float = -10000.0
## A track's event slots by the key the `.scn` gives them (Track.cpp:815-858, 952-954)
const TRACK_EVENTS:Dictionary[String, ScenarioEventServer.TrackEvent] = {
    "event0": ScenarioEventServer.TRACK_EVENT0,
    "event1": ScenarioEventServer.TRACK_EVENT1,
    "event2": ScenarioEventServer.TRACK_EVENT2,
    "eventall0": ScenarioEventServer.TRACK_EVENTALL0,
    "eventall1": ScenarioEventServer.TRACK_EVENTALL1,
    "eventall2": ScenarioEventServer.TRACK_EVENTALL2,
}
## `sound <mode>`: 1 plays, -1 loops, 0 stops (Event.cpp:1379-1390)
const SOUND_MODES:Dictionary[int, MaszynaLegacySoundAction.Mode] = {
    0: MaszynaLegacySoundAction.Mode.STOP,
    1: MaszynaLegacySoundAction.Mode.PLAY,
    -1: MaszynaLegacySoundAction.Mode.LOOP,
}
## `animation <rotate|translate> <submodel> <x> <y> <z> <speed>` (Event.cpp:1606-1682)
const ANIMATION_MODES:Dictionary[String, MaszynaLegacyAnimationAction.Mode] = {
    "rotate": MaszynaLegacyAnimationAction.MODE_ROTATE,
    "translate": MaszynaLegacyAnimationAction.MODE_TRANSLATE,
}


static func build(
    root:MaszynaIncludeNode,
    events:Array[MaszynaEventData],
    memcells:Array[MaszynaMemcellData],
    launchers:Array[MaszynaEventLauncherData],
    sounds:Array[MaszynaSoundData],
    tracks:Array[MaszynaTrackData],
    track_rids:Array[RID],
    models:Array[MaszynaModelData],
    model_rids:Array[RID],
    power_sources:Array[MaszynaPowerSourceData],
) -> void:
    # the tracks and models come with the RIDs they were built as, in their order
    var tracks_by_name:Dictionary[String, RID] = {}
    for index:int in tracks.size():
        if tracks[index].track_name:
            tracks_by_name[tracks[index].track_name.to_lower()] = track_rids[index]
    var model_names:Dictionary[String, String] = {}
    var instances_by_name:Dictionary[String, RID] = {}
    for index:int in models.size():
        model_names[models[index].name.to_lower()] = models[index].name
        instances_by_name[models[index].name.to_lower()] = model_rids[index]
    var power_source_names:Dictionary[String, String] = {}
    for power_source_data:MaszynaPowerSourceData in power_sources:
        power_source_names[power_source_data.name.to_lower()] = power_source_data.name

    var memories:Dictionary[String, RID] = {}
    for memcell:MaszynaMemcellData in memcells:
        var memory:RID = ScenarioEventServer.memory_create()
        root._memory_rids.append(memory)
        ScenarioEventServer.memory_set_name(memory, memcell.name)
        ScenarioEventServer.memory_set_values(memory, memcell.text, memcell.value1, memcell.value2)
        if memcell.track:
            ScenarioEventServer.memory_set_track(memory, tracks_by_name.get(memcell.track, RID()))
        memories[memcell.name.to_lower()] = memory

    # a scenery sound is a player of its own with a bank of its one file, played once or looped
    var players_by_name:Dictionary[String, SfxPlayer3D] = {}
    for sound:MaszynaSoundData in sounds:
        var bank:SfxBank = SfxBank.new()
        var bank_events:Array[SfxEvent] = [
            _build_sound_event(MaszynaLegacySoundAction.PLAY_EVENT, sound.file, false),
            _build_sound_event(MaszynaLegacySoundAction.LOOP_EVENT, sound.file, true),
        ]
        bank.events = bank_events
        var player:SfxPlayer3D = SfxPlayer3D.new()
        player.name = sound.name if sound.name else "sound"
        player.bank = bank
        player.max_distance = int(sound.range_max)
        root.add_child(player)
        player.global_position = sound.position
        players_by_name[sound.name.to_lower()] = player

    # every event exists before any refers to another
    var event_rids:Array[RID] = []
    var events_by_name:Dictionary[String, RID] = {}
    for event:MaszynaEventData in events:
        var rid:RID = ScenarioEventServer.event_create()
        root._event_rids.append(rid)
        ScenarioEventServer.event_set_name(rid, event.name)
        ScenarioEventServer.event_set_delay(rid, absf(event.delay))
        ScenarioEventServer.event_set_random_delay(rid, event.random_delay)
        event_rids.append(rid)
        events_by_name[event.name] = rid

    for index:int in events.size():
        var event:MaszynaEventData = events[index]
        var rid:RID = event_rids[index]
        var event_memories:Array[RID] = _get_memories(event.targets, memories)
        var targets:Array[RID] = []
        match event.type:
            "updatevalues", "addvalues":
                var action:MaszynaLegacyMemoryAction = MaszynaLegacyMemoryAction.new()
                action.memories = event_memories
                action.mask = _fields_mask(event.parameters)
                action.text = event.parameters[0]
                action.value1 = float(event.parameters[1])
                action.value2 = float(event.parameters[2])
                action.mode = MaszynaLegacyMemoryAction.MODE_ADD if event.type == "addvalues" else MaszynaLegacyMemoryAction.MODE_SET
                ScenarioEventServer.event_attach_action(rid, action)
            "copyvalues":
                var action:MaszynaLegacyMemoryAction = MaszynaLegacyMemoryAction.new()
                action.memories = event_memories
                action.source = memories.get(event.parameters[0].to_lower(), RID())
                action.mask = int(event.parameters[1]) if event.parameters.size() > 1 else ALL_FIELDS
                ScenarioEventServer.event_attach_action(rid, action)
            "multiple":
                var children:Array[RID] = []
                var else_children:Array[RID] = []
                var after_else:bool = false
                for child:String in event.parameters:
                    var child_name:String = child.to_lower()
                    if child_name == ELSE:
                        after_else = not after_else
                        continue
                    if not events_by_name.has(child_name):
                        continue
                    if after_else:
                        else_children.append(events_by_name[child_name])
                    else:
                        children.append(events_by_name[child_name])
                var action:MaszynaLegacyMultipleAction = MaszynaLegacyMultipleAction.new()
                action.events = children
                action.else_events = else_children
                ScenarioEventServer.event_attach_action(rid, action)
            "lights":
                var aspects:Array[StringName] = []
                for target:String in event.targets:
                    var semaphore:RID = SemaphoreServer.semaphore_get_rid_by_name(model_names.get(target, ""))
                    if not semaphore.is_valid():
                        continue
                    targets.append(semaphore)
                    aspects.append(MaszynaLegacySemaphoreKindFactory.get_aspect_name(event.name, target))
                var action:MaszynaLegacyLightsAction = MaszynaLegacyLightsAction.new()
                action.semaphores = targets
                action.aspects = aspects
                ScenarioEventServer.event_attach_action(rid, action)
            "switch":
                for target:String in event.targets:
                    if tracks_by_name.has(target):
                        targets.append(tracks_by_name[target])
                var action:MaszynaLegacySwitchAction = MaszynaLegacySwitchAction.new()
                action.tracks = targets
                # the state is 0 or 1 (TTrack::Switch(), Track.cpp:1752)
                action.active_track = TrackManager.TRACK_DIVERGING if int(event.parameters[0]) == TrackManager.TRACK_DIVERGING else TrackManager.TRACK_COMMON
                ScenarioEventServer.event_attach_action(rid, action)
            "trackvel":
                for target:String in event.targets:
                    if tracks_by_name.has(target):
                        targets.append(tracks_by_name[target])
                var action:MaszynaLegacyTrackVelocityAction = MaszynaLegacyTrackVelocityAction.new()
                action.tracks = targets
                action.velocity = float(event.parameters[0])
                ScenarioEventServer.event_attach_action(rid, action)
            "voltage":
                for target:String in event.targets:
                    var power_source:RID = TractionPowerServer.power_source_get_rid_by_name(power_source_names.get(target, ""))
                    if power_source.is_valid():
                        targets.append(power_source)
                var action:MaszynaLegacyVoltageAction = MaszynaLegacyVoltageAction.new()
                action.power_sources = targets
                action.voltage = float(event.parameters[0])
                ScenarioEventServer.event_attach_action(rid, action)
            "sound":
                var players:Array[SfxPlayer3D] = []
                for target:String in event.targets:
                    if players_by_name.has(target):
                        players.append(players_by_name[target])
                var action:MaszynaLegacySoundAction = MaszynaLegacySoundAction.new()
                action.players = players
                action.mode = SOUND_MODES.get(int(event.parameters[0]), MaszynaLegacySoundAction.Mode.STOP)
                ScenarioEventServer.event_attach_action(rid, action)
            "animation":
                var mode:String = event.parameters[0].to_lower()
                if not ANIMATION_MODES.has(mode):
                    continue
                for target:String in event.targets:
                    if instances_by_name.has(target) and instances_by_name[target].is_valid():
                        targets.append(instances_by_name[target])
                var action:MaszynaLegacyAnimationAction = MaszynaLegacyAnimationAction.new()
                action.instances = targets
                action.mode = ANIMATION_MODES[mode]
                action.submodel = event.parameters[1]
                action.target = Vector3(float(event.parameters[2]), float(event.parameters[3]), float(event.parameters[4]))
                action.speed = float(event.parameters[5])
                ScenarioEventServer.event_attach_action(rid, action)

        # the targets of an event are the memories its `memcompare` reads (Event.cpp:1213-1233)
        var condition:MaszynaLegacyEventCondition = MaszynaLegacyEventCondition.new()
        var conditioned:bool = false
        var position:int = 0
        while position < event.condition.size():
            var keyword:String = event.condition[position].to_lower()
            position += 1
            if keyword == "memcompare":
                _set_memcompare(condition, event_memories, event.condition.slice(position, position + MEMCOMPARE_FIELDS))
                position += MEMCOMPARE_FIELDS
                conditioned = true
            elif keyword == "probability" or keyword == "propability":
                condition.probability = float(event.condition[position])
                position += 1
                conditioned = true
            # memcompareex, trackfree and trackoccupied are not ported (TODO.md)
        if conditioned:
            ScenarioEventServer.event_attach_condition(rid, condition)

    # the events a track names for its slots, and the ones named after the track and the slot
    for index:int in tracks.size():
        var track_data:MaszynaTrackData = tracks[index]
        var bound:bool = false
        for key:String in TRACK_EVENTS:
            var names:PackedStringArray = [str(track_data.parameters.get(key, "")).to_lower()]
            if track_data.track_name:
                names.append(track_data.track_name.to_lower() + ":" + key)
            for event_name:String in names:
                if events_by_name.has(event_name):
                    ScenarioEventServer.track_add_event(track_rids[index], TRACK_EVENTS[key], events_by_name[event_name])
                    bound = true
        if bound:
            root._event_track_rids.append(track_rids[index])

    for launcher_data:MaszynaEventLauncherData in launchers:
        var launcher:RID = ScenarioEventServer.launcher_create()
        root._launcher_rids.append(launcher)
        ScenarioEventServer.launcher_set_name(launcher, launcher_data.name)
        ScenarioEventServer.launcher_set_position(launcher, launcher_data.position)
        ScenarioEventServer.launcher_set_radius(launcher, launcher_data.radius)
        ScenarioEventServer.launcher_set_events(
            launcher, events_by_name.get(launcher_data.event1, RID()), events_by_name.get(launcher_data.event2, RID())
        )
        # radio calls and key codes are not ported (TODO.md)
        if launcher_data.key.length() == 1:
            ScenarioEventServer.launcher_set_key(launcher, OS.find_keycode_from_string(launcher_data.key.to_upper()))
        if launcher_data.condition:
            var condition:MaszynaLegacyEventCondition = MaszynaLegacyEventCondition.new()
            _set_memcompare(
                condition, _get_memories(launcher_data.condition.slice(0, 1), memories), launcher_data.condition.slice(1)
            )
            ScenarioEventServer.launcher_attach_condition(launcher, condition)
        if launcher_data.delta_time > 0.0:
            var hhmm:int = int(launcher_data.delta_time)
            ScenarioEventServer.launcher_set_time_of_day(launcher, floori(hhmm / float(HHMM_HOUR)), hhmm % HHMM_HOUR)
        elif launcher_data.delta_time < 0.0 and not launcher_data.delta_time == FIRE_ONCE_IN_RANGE:
            ScenarioEventServer.launcher_set_interval(launcher, -launcher_data.delta_time)

    # queued at the start: a negative delay (InitEvents(), Event.cpp:2493-2499) and the first event
    # of each name containing "onstart" (Event.cpp:2336-2345)
    var started:Dictionary[String, bool] = {}
    for index:int in events.size():
        var event:MaszynaEventData = events[index]
        var onstart:bool = ONSTART in event.name and not started.has(event.name)
        started[event.name] = true
        if event.delay < 0.0 or onstart:
            ScenarioEventServer.event_queue(event_rids[index])


## The fields a `<text> <value1> <value2>` triple sets or compares - all but those given as `*`
static func _fields_mask(fields:PackedStringArray) -> int:
    var mask:int = 0
    if not fields[0] == FIELD_KEPT:
        mask |= ScenarioEventServer.MEMORY_FIELD_TEXT
    if not fields[1] == FIELD_KEPT:
        mask |= ScenarioEventServer.MEMORY_FIELD_VALUE1
    if not fields[2] == FIELD_KEPT:
        mask |= ScenarioEventServer.MEMORY_FIELD_VALUE2
    return mask


static func _get_memories(names:PackedStringArray, memories:Dictionary[String, RID]) -> Array[RID]:
    var found:Array[RID] = []
    for memory_name:String in names:
        if memories.has(memory_name):
            found.append(memories[memory_name])
    return found


## `memcompare <text> <value1> <value2>` - a memory that does not exist passes, as in the original
## (Event.cpp:131-134), so it is simply left out
static func _set_memcompare(
    condition:MaszynaLegacyEventCondition, compared:Array[RID], fields:PackedStringArray
) -> void:
    condition.memories = compared
    condition.mask = _fields_mask(fields)
    condition.text = fields[0]
    condition.value1 = float(fields[1])
    condition.value2 = float(fields[2])


static func _build_sound_event(event_name:StringName, file:String, loop:bool) -> SfxEvent:
    var stream:MaszynaAudioStream = MaszynaAudioStream.new()
    stream.file_path = file
    stream.loop = loop
    var clip:SfxClip = SfxClip.new()
    clip.stream = stream
    var event:SfxEvent = SfxEvent.new()
    event.name = event_name
    var clips:Array[SfxClip] = [clip]
    event.clips = clips
    return event
