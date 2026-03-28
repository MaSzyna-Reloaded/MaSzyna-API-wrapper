@tool
extends Node3D
class_name MaszynaSwitch3D

enum SwitchState { STRAIGHT, DIVERGING }

@export var state: SwitchState = SwitchState.STRAIGHT:
    set(x):
        state = x
        _update_switch()

@export var straight_track: MaszynaTrack3D:
    set(x):
        straight_track = x
        update_configuration_warnings()
        _update_switch()

@export var diverging_track: MaszynaTrack3D:
    set(x):
        diverging_track = x
        update_configuration_warnings()
        _update_switch()

@export var common_track: MaszynaTrack3D:
    set(x):
        common_track = x
        update_configuration_warnings()
        _update_switch()

func _ready():
    _update_switch()

func _get_configuration_warnings():
    var warnings = []
    if not straight_track:
        warnings.append("No straight track node assigned.")
    if not diverging_track:
        warnings.append("No diverging track node assigned.")
    if not common_track:
        warnings.append("No common track node assigned.")
    
    if straight_track and diverging_track and common_track:
        if not straight_track.curve or not diverging_track.curve or not common_track.curve:
            warnings.append("One or more tracks are missing curves.")
        else:
            var inlet_pos = _get_inlet_position()
            var s_start = straight_track.to_global(straight_track.curve.get_point_position(0))
            var s_end = straight_track.to_global(straight_track.curve.get_point_position(straight_track.curve.point_count - 1))
            
            if s_start.distance_to(inlet_pos) > 0.5 and s_end.distance_to(inlet_pos) > 0.5:
                warnings.append("Straight track does not seem to connect to the switch inlet.")
    
    return warnings

func _get_inlet_position() -> Vector3:
    if common_track and common_track.curve:
        # inlet is at the end of common track (usually)
        return common_track.to_global(common_track.curve.get_point_position(common_track.curve.point_count - 1))
    if straight_track and straight_track.curve:
        # or at the start of straight track
        return straight_track.to_global(straight_track.curve.get_point_position(0))
    return global_position

func resolve_track(incoming_track: Node3D) -> MaszynaTrack3D:
    if incoming_track == common_track:
        return straight_track if state == SwitchState.STRAIGHT else diverging_track
    elif incoming_track == straight_track or incoming_track == diverging_track:
        return common_track
    return null

func _update_switch():
    if not is_inside_tree(): return
    update_configuration_warnings()
    
    if common_track: _auto_link_track(common_track)
    if straight_track: _auto_link_track(straight_track)
    if diverging_track: _auto_link_track(diverging_track)

func _auto_link_track(track: MaszynaTrack3D):
    if not track or not track.curve: return
    var inlet_pos = _get_inlet_position()
    var p_start = track.to_global(track.curve.get_point_position(0))
    var p_end = track.to_global(track.curve.get_point_position(track.curve.point_count - 1))
    
    # Only link the end that is actually AT the switch junction
    if p_start.distance_to(inlet_pos) < 0.5:
        if track.previous_track.is_empty():
            track.previous_track = track.get_path_to(self)
    elif p_end.distance_to(inlet_pos) < 0.5:
        if track.next_track.is_empty():
            track.next_track = track.get_path_to(self)

func toggle():
    state = SwitchState.DIVERGING if state == SwitchState.STRAIGHT else SwitchState.STRAIGHT
