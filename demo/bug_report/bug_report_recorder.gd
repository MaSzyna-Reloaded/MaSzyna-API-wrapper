class_name BugReportRecorder
extends Node

## What a problem report cannot read back at the moment it is made: the commands the player's
## vehicle received and the scenario's events that ran, from start() to stop(), the oldest dropped
## past their limits. A command carries no sender, so the scenario's commands to the player's
## vehicle are kept as well as the player's own.

## Commands kept, the latest
const MAX_COMMANDS: int = 500
## Events that ran kept, the latest
const MAX_LAUNCHED_EVENTS: int = 200

## "<time> <simulation time> <vehicle> <command> <p1> <p2>", one line a command
var _commands: PackedStringArray = []
## "<time> <simulation time> <event> <activator>", one line an event
var _launched_events: PackedStringArray = []


## For a scenery being started
func start() -> void:
    VehicleServer.vehicle_command_received.connect(_on_vehicle_command_received)
    ScenarioEventServer.event_launched.connect(_on_event_launched)


## For a scenery being left: what it kept goes with it
func stop() -> void:
    VehicleServer.vehicle_command_received.disconnect(_on_vehicle_command_received)
    ScenarioEventServer.event_launched.disconnect(_on_event_launched)
    _commands.clear()
    _launched_events.clear()


## The commands the player's vehicle received, a line each, the oldest first
func get_commands() -> PackedStringArray:
    return _commands


## The scenario's events that ran, a line each, the oldest first
func get_launched_events() -> PackedStringArray:
    return _launched_events


func _on_vehicle_command_received(vehicle_rid: RID, command: String, p1: Variant, p2: Variant) -> void:
    if not vehicle_rid == PlayerServer.player_get_vehicle():
        return
    if _commands.size() == MAX_COMMANDS:
        _commands.remove_at(0)
    _commands.append("%s %.3f %s %s %s %s" % [
        Time.get_time_string_from_system(), SimulationServer.simulation_get_time(),
        VehicleServer.vehicle_get_name(vehicle_rid), command, p1, p2
    ])


func _on_event_launched(event: RID, activator: RID) -> void:
    if _launched_events.size() == MAX_LAUNCHED_EVENTS:
        _launched_events.remove_at(0)
    _launched_events.append("%s %.3f %s %s" % [
        Time.get_time_string_from_system(), SimulationServer.simulation_get_time(),
        ScenarioEventServer.event_get_name(event), VehicleServer.vehicle_get_name(activator)
    ])
