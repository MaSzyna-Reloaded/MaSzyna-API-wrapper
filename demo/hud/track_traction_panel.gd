extends VBoxContainer

## Where the driven vehicle is and what it is collecting from: the track under it with its own
## name and offset, and, per pantograph, the wire overhead with its height and voltage.
##
## Written to answer one question in the field - at which point of a scenery does a vehicle lose
## the line, and is it losing the wire or finding a wire that carries nothing. The wire query is
## the same TractionPowerServer call the vehicle itself makes, so what the panel shows is what the
## vehicle sees, not a second opinion.

## Seconds between refreshes. Faster than the streaming panel's: a loss of contact lasts a few
## frames, and a readout that only looks four times a second would walk straight past it.
const REFRESH_INTERVAL:float = 0.1

## Half of the slider's width when the vehicle has no electric engine to declare its CSW.
const FALLBACK_SLIDER_HALF_WIDTH:float = 0.5
## DynObj.cpp:93 fWidthExtra - the guide horn beyond the slider.
const HORN_WIDTH:float = 0.381

@export var player_path:NodePath

var _rows:Dictionary[String, Label] = {}
var _elapsed:float = 0.0
## Lowest voltage seen since the last reset, with where it was - a drop of three frames is gone
## from the live readout before anyone can read it.
var _worst_voltage:float = INF
var _worst_at:Vector3 = Vector3.ZERO
var _worst_track:String = ""


func _ready() -> void:
    for caption:String in [
        "Vehicle", "Track", "Offset", "Track length", "Switch", "Position",
        "Pantograph 1", "Wire 1", "Pantograph 2", "Wire 2", "Slider", "Worst drop",
    ]:
        _rows[caption] = _add_row(caption)
    _refresh()


func _process(delta:float) -> void:
    _elapsed += delta
    if _elapsed < REFRESH_INTERVAL:
        return
    _elapsed = 0.0
    _refresh()


## Clears the remembered drop, so the next run over a suspect spot is read on its own.
func reset_worst_drop() -> void:
    _worst_voltage = INF
    _worst_at = Vector3.ZERO
    _worst_track = ""


func _refresh() -> void:
    var player:MaszynaPlayer = get_node_or_null(player_path) as MaszynaPlayer
    var vehicle:RailVehicle3D = player.controlled_vehicle if player else null
    if not vehicle:
        _rows["Vehicle"].text = "none"
        return
    _rows["Vehicle"].text = vehicle.name

    var rid:RID = vehicle.get_rid()
    var placement:Dictionary = RailVehicleServer.vehicle_get_track_position(rid)
    var track:RID = placement["track_rid"]
    if track.is_valid():
        var track_name:String = TrackManager.track_get_name(track)
        _rows["Track"].text = track_name if track_name else "(unnamed)"
        _rows["Offset"].text = "%.2f m" % float(placement["along"])
        _rows["Track length"].text = "%.2f m" % TrackManager.track_get_length(track)
        _rows["Switch"].text = (
            "yes, branch %d%s" % [
                TrackManager.switch_get_active_track(track),
                " (right)" if TrackManager.switch_is_right(track) else " (left)",
            ]
            if TrackManager.track_is_switch(track) else "no"
        )
    else:
        _rows["Track"].text = "none"
        _rows["Offset"].text = "-"
        _rows["Track length"].text = "-"
        _rows["Switch"].text = "-"

    var origin:Vector3 = vehicle.global_position
    _rows["Position"].text = "%.1f, %.1f, %.1f" % [origin.x, origin.y, origin.z]

    var engine:VehicleElectricEngine = RailVehicleServer.vehicle_component_get(
            rid, VehicleComponentType.COMPONENT_ENGINE) as VehicleElectricEngine
    var half_width:float = FALLBACK_SLIDER_HALF_WIDTH
    if engine:
        var sliding_width:float = engine.power_current_collector_sliding_width
        if sliding_width > 0.0:
            half_width = 0.5 * sliding_width
    _rows["Slider"].text = "%.3f m half width + %.3f m horn" % [half_width, HORN_WIDTH]

    var state:Dictionary = RailVehicleServer.vehicle_dump_state(rid)
    _report_pantograph(
            vehicle, 1, vehicle.pantograph_front_offset, half_width,
            bool(state.get("current_collector/pantograph_first_active", false)),
            float(state.get("current_collector/pantograph_first_voltage", 0.0)))
    _report_pantograph(
            vehicle, 2, vehicle.pantograph_rear_offset, half_width,
            bool(state.get("current_collector/pantograph_second_active", false)),
            float(state.get("current_collector/pantograph_second_voltage", 0.0)))

    if _worst_voltage < INF:
        _rows["Worst drop"].text = "%.0f V on %s at %.0f, %.0f, %.0f" % [
            _worst_voltage, _worst_track, _worst_at.x, _worst_at.y, _worst_at.z,
        ]
    else:
        _rows["Worst drop"].text = "none yet"


func _report_pantograph(
        vehicle:RailVehicle3D, number:int, offset:Vector3, half_width:float,
        is_active:bool, voltage:float) -> void:
    var pantograph_row:Label = _rows["Pantograph %d" % number]
    var wire_row:Label = _rows["Wire %d" % number]
    if not is_active:
        pantograph_row.text = "down"
        wire_row.text = "-"
        return
    pantograph_row.text = "up, %.0f V" % voltage

    var transform:Transform3D = vehicle.global_transform
    var contact_point:Vector3 = transform * offset
    var found:Dictionary = TractionPowerServer.wire_find_above_with_height(
            contact_point, transform.basis.y, -transform.basis.z, -transform.basis.x,
            half_width, HORN_WIDTH)
    var wire:RID = found["rid"]
    if not wire.is_valid():
        wire_row.text = "NO WIRE in reach"
        _remember_drop(vehicle, 0.0)
        return
    var wire_voltage:float = TractionPowerServer.wire_get_voltage(wire, voltage, 0.0)
    wire_row.text = "%.2f m above, %.0f V" % [float(found["height"]), wire_voltage]
    _remember_drop(vehicle, wire_voltage)


func _remember_drop(vehicle:RailVehicle3D, voltage:float) -> void:
    if voltage >= _worst_voltage:
        return
    _worst_voltage = voltage
    _worst_at = vehicle.global_position
    var placement:Dictionary = RailVehicleServer.vehicle_get_track_position(vehicle.get_rid())
    var track:RID = placement["track_rid"]
    _worst_track = TrackManager.track_get_name(track) if track.is_valid() else "(no track)"


func _add_row(caption:String) -> Label:
    var row := HBoxContainer.new()
    var caption_label := Label.new()
    caption_label.text = caption
    caption_label.custom_minimum_size = Vector2(110, 0)
    var value_label := Label.new()
    value_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    value_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
    row.add_child(caption_label)
    row.add_child(value_label)
    add_child(row)
    return value_label
