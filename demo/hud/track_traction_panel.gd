extends VBoxContainer

## Where the driven vehicle is and what it is collecting from: the track under it with its own
## name and offset, and, per pantograph, the wire overhead with its height and voltage.
##
## Catching a loss of contact is deliberately NOT this panel's job. It lasts two or three frames -
## measured on zwierzyniec_tlk - so watching for it here would mean running GDScript every frame
## for something the vehicle already sees. RailVehicle3D warns with the place the moment it
## happens; this is the live readout beside those warnings.

## Seconds between refreshes. Only the live rows change with it, and only while the window is open.
const REFRESH_INTERVAL:float = 0.25

## Half of the slider's width when the vehicle has no electric engine to declare its CSW.
const FALLBACK_SLIDER_HALF_WIDTH:float = 0.5
## DynObj.cpp:93 fWidthExtra - the guide horn beyond the slider.
const HORN_WIDTH:float = 0.381

@export var player_path:NodePath

var _rows:Dictionary[String, Label] = {}
var _elapsed:float = 0.0
## Taken once per vehicle rather than looked up per refresh; null for anything that is not
## electric, and then the traction rows have nothing to say.
var _engine:VehicleElectricEngine = null
var _engine_vehicle:RailVehicle3D = null


func _ready() -> void:
    for caption:String in [
        "Vehicle", "Track", "Offset", "Track length", "Switch", "Position",
        "Pantograph 1", "Wire 1", "Pantograph 2", "Wire 2", "Slider",
    ]:
        _rows[caption] = _add_row(caption)
    set_process(is_visible_in_tree())
    _refresh()


## A debug window costs nothing while it is closed.
func _notification(what:int) -> void:
    if what == NOTIFICATION_VISIBILITY_CHANGED:
        set_process(is_visible_in_tree())


func _process(delta:float) -> void:
    _elapsed += delta
    if _elapsed < REFRESH_INTERVAL:
        return
    _elapsed = 0.0
    _refresh()


func _refresh() -> void:
    var player:MaszynaPlayer = get_node_or_null(player_path) as MaszynaPlayer
    var vehicle:RailVehicle3D = player.controlled_vehicle if player else null
    if not vehicle:
        _rows["Vehicle"].text = "none"
        _engine = null
        _engine_vehicle = null
        return

    var rid:RID = vehicle.get_rid()
    var controller:VehicleController = vehicle.get_controller()
    var train_id:String = controller.train_id if controller else ""
    _rows["Vehicle"].text = train_id if train_id else "(no train id)"

    var placement:Dictionary = RailVehicleServer.vehicle_get_track_position(rid)
    var track:RID = placement["track_rid"]
    if track.is_valid():
        var track_name:String = TrackManager.track_get_name(track)
        _rows["Track"].text = track_name if track_name else "(unnamed)"
        _rows["Offset"].text = "%.2f m" % float(placement["along"])
        _rows["Track length"].text = "%.2f m" % TrackManager.track_get_length(track)
        _rows["Switch"].text = (
            tr("yes, branch %d%s") % [
                TrackManager.switch_get_active_track(track),
                tr(" (right)") if TrackManager.switch_is_right(track) else tr(" (left)"),
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

    if not _engine_vehicle == vehicle:
        _engine_vehicle = vehicle
        _engine = RailVehicleServer.vehicle_component_get(
                rid, VehicleComponentType.COMPONENT_ENGINE) as VehicleElectricEngine
    if not _engine:
        _rows["Slider"].text = "-"
        for number:int in [1, 2]:
            _rows["Pantograph %d" % number].text = "not an electric vehicle"
            _rows["Wire %d" % number].text = "-"
        return

    var half_width:float = FALLBACK_SLIDER_HALF_WIDTH
    var sliding_width:float = _engine.power_current_collector_sliding_width
    if sliding_width > 0.0:
        half_width = 0.5 * sliding_width
    _rows["Slider"].text = tr("%.3f m half width + %.3f m horn") % [half_width, HORN_WIDTH]

    _report_pantograph(
            vehicle, 1, vehicle.pantograph_front_offset, half_width,
            _engine.get_collector_pantograph_first_active(),
            _engine.get_collector_pantograph_first_voltage())
    _report_pantograph(
            vehicle, 2, vehicle.pantograph_rear_offset, half_width,
            _engine.get_collector_pantograph_second_active(),
            _engine.get_collector_pantograph_second_voltage())


func _report_pantograph(
        vehicle:RailVehicle3D, number:int, offset:Vector3, half_width:float,
        is_active:bool, voltage:float) -> void:
    var pantograph_row:Label = _rows["Pantograph %d" % number]
    var wire_row:Label = _rows["Wire %d" % number]
    if not is_active:
        pantograph_row.text = "down"
        wire_row.text = "-"
        return
    pantograph_row.text = tr("up, %.0f V") % voltage

    var transform:Transform3D = vehicle.global_transform
    var contact_point:Vector3 = transform * offset
    var found:Dictionary = TractionPowerServer.wire_find_above_with_height(
            contact_point, transform.basis.y, -transform.basis.z, -transform.basis.x,
            half_width, HORN_WIDTH)
    var wire:RID = found["rid"]
    if not wire.is_valid():
        wire_row.text = "NO WIRE in reach"
        return
    wire_row.text = tr("%.2f m above, %.0f V") % [
        float(found["height"]), TractionPowerServer.wire_get_voltage(wire, voltage, 0.0),
    ]


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
