class_name TimetableRow
extends PanelContainer

## One station of the timetable panel: where it lies on the line, when the train is due there and
## how far the train has got

## Where the train is against this station
enum Progress { PASSED, NEXT, AHEAD }
## Where the station stands in the timetable - the line is drawn only towards its neighbours
enum Place { FIRST, BETWEEN, LAST }
## What the status of the next station tells the driver
enum Tone { NEUTRAL, WAIT, GO, EXCHANGE }

## The speed shown beside a station where it does not change from the station before
const NO_SPEED:float = -1.0
const MINUTES_PER_HOUR:float = 60.0
const HOURS_PER_DAY:int = 24
const ACCENT:Color = Color(0.45, 0.72, 1.0)
const LINE_AHEAD:Color = Color(0.6, 0.66, 0.75, 0.45)
const DOT_AHEAD:Color = Color(0.75, 0.8, 0.88)
## The card's own background, filling the ring of a station ahead over the line
const CARD_BACKGROUND:Color = Color(0.06, 0.075, 0.1)
## Passed stations fade behind the ones still to come
const PASSED_ALPHA:float = 0.45
const LINE_WIDTH:float = 2.0
## Between the two lines drawn for a double-track line
const DOUBLE_TRACK_GAP:float = 4.0
const STOP_RADIUS:float = 6.0
const PASS_RADIUS:float = 3.5
const RING_WIDTH:float = 2.0
const TONE_COLORS:Dictionary[Tone, Color] = {
    Tone.NEUTRAL: Color(0.45, 0.72, 1.0),
    Tone.WAIT: Color(1.0, 0.72, 0.35),
    Tone.GO: Color(0.4, 0.85, 0.55),
    # red while the passengers get off and on (driveruipanels.cpp:440)
    Tone.EXCHANGE: Color(1.0, 0.4, 0.35),
}
## A pass-through station is named in the muted colour
const PASS_THROUGH_COLOR:Color = Color(0.72, 0.77, 0.85)

var _entry:TimetableEntry = null
var _place:Place = Place.BETWEEN
var _progress:Progress = Progress.AHEAD


## The station, its place in the timetable and the speed shown beside it, or NO_SPEED
func show_entry(entry:TimetableEntry, place:Place, velocity:float) -> void:
    _entry = entry
    _place = place
    %StationName.text = entry.station_name.replace("_", " ")
    if not entry.is_stop():
        %StationName.add_theme_color_override(&"font_color", PASS_THROUGH_COLOR)
    %Kilometre.text = "%.2f km" % entry.kilometre
    %Facilities.text = entry.facilities.replace(",", " · ")
    %Speed.text = "%d" % velocity
    %SpeedChip.visible = velocity > 0.0
    %Arrival.text = format_time(entry.arrival)
    %Arrival.visible = entry.is_stop() and entry.arrival >= 0.0
    %Departure.text = format_time(entry.departure)
    %Departure.visible = entry.departure >= 0.0
    if not entry.is_stop():
        %Departure.theme_type_variation = &"TimetableMuted"
    %Line.queue_redraw()


func show_progress(progress:Progress) -> void:
    _progress = progress
    theme_type_variation = &"TimetableRowCurrent" if progress == Progress.NEXT else &"TimetableRow"
    modulate.a = PASSED_ALPHA if progress == Progress.PASSED else 1.0
    %Status.visible = false
    %Line.queue_redraw()


## The status of the next station, beside its name
func show_status(text:String, tone:Tone) -> void:
    %StatusText.text = text
    %StatusText.add_theme_color_override(&"font_color", TONE_COLORS[tone])
    %Status.visible = true


## The line through the station and its dot (the Line node's draw signal, a [connection] in the
## scene): full for a stop, small for a station passed through; filled once passed, a ring for the
## next one
func _draw_line() -> void:
    var line:Control = %Line
    var middle:Vector2 = Vector2(line.size.x / 2.0, %StationName.position.y + %StationName.size.y / 2.0)
    # through the row's own margins, to meet the line of the rows above and below
    var panel:StyleBox = get_theme_stylebox(&"panel")
    var top:float = middle.y if _place == Place.FIRST else -panel.get_margin(SIDE_TOP)
    var bottom:float = middle.y if _place == Place.LAST else line.size.y + panel.get_margin(SIDE_BOTTOM)
    var color:Color = ACCENT if _progress == Progress.PASSED else LINE_AHEAD
    var offsets:Array[float] = [0.0]
    if _entry and _entry.track_count > 1:
        offsets = [-DOUBLE_TRACK_GAP / 2.0, DOUBLE_TRACK_GAP / 2.0]
    for offset:float in offsets:
        line.draw_line(Vector2(middle.x + offset, top), Vector2(middle.x + offset, bottom), color, LINE_WIDTH)
    var radius:float = STOP_RADIUS if _entry and _entry.is_stop() else PASS_RADIUS
    match _progress:
        Progress.PASSED:
            line.draw_circle(middle, radius, ACCENT)
        Progress.NEXT:
            line.draw_circle(middle, radius + RING_WIDTH, ACCENT)
            line.draw_circle(middle, radius - RING_WIDTH / 2.0, Color.WHITE)
        Progress.AHEAD:
            line.draw_circle(middle, radius, CARD_BACKGROUND)
            line.draw_circle(middle, radius, DOT_AHEAD, false, RING_WIDTH)


## Hours since midnight as HH:MM
static func format_time(hours:float) -> String:
    var minutes:int = roundi(hours * MINUTES_PER_HOUR)
    return "%02d:%02d" % [(minutes / int(MINUTES_PER_HOUR)) % HOURS_PER_DAY, minutes % int(MINUTES_PER_HOUR)]
