@tool
extends RefCounted

## `event <name> <type> <delay> <targets> <parameters> endevent` (Event.cpp:289-333). Only a
## `lights` event is kept - it is an aspect of the semaphores it is aimed at; the body of any
## other type is read through to its `endevent` and dropped (see TODO.md).

## Events named "none_..." are ignored by the original (Event.cpp:301)
const IGNORED_PREFIX:String = "none_"
## Targets are separated by either and read in lower case; "none" is no target (Event.cpp:327-329)
const TARGET_SEPARATORS:Array[String] = ["|", ","]
const NO_TARGET:String = "none"


func import(p:MaszynaParser, context: MaszynaImporterContext):
    var name:String = p.next_token().to_lower()
    var type:String = p.next_token().to_lower()
    if not type == "lights" or name.begins_with(IGNORED_PREFIX):
        while not p.eof_reached():
            if p.next_token().to_lower() == "endevent":
                break
        return []

    var _delay:String = p.next_token()
    var targets:PackedStringArray = [p.next_token().to_lower()]
    for separator:String in TARGET_SEPARATORS:
        var split:PackedStringArray = []
        for part:String in targets:
            split.append_array(part.split(separator, false))
        targets = split
    var none_index:int = targets.find(NO_TARGET)
    while not none_index == -1:
        targets.remove_at(none_index)
        none_index = targets.find(NO_TARGET)

    var values:PackedFloat32Array = []
    while not p.eof_reached():
        var token:String = p.next_token()
        var keyword:String = token.to_lower()
        if keyword == "endevent":
            break
        if keyword == "randomdelay":
            p.next_token()
            continue
        values.append(float(token))

    var event := MaszynaLightsEventData.new()
    event.name = name
    event.targets = targets
    event.values = values
    context.light_events.append(event)
    return []
