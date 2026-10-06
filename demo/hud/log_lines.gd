class_name LogLines
extends RichTextLabel

## A log to scroll back through: the newest line at the bottom, followed while the view stands
## there. Only the last MAX_LINES are kept.

## Lines kept; the oldest go first, so a long session does not grow the text without end
const MAX_LINES: int = 5000
## The HUD's text colour (Label in timetable_theme.tres)
const TEXT_COLOR: Color = Color(0.9, 0.93, 0.97, 1)

## Lines in the view, to know when the oldest goes
var _line_count: int = 0


## A line at the bottom, added as text, so a bracket in it is not read as BBCode
func add_line(line: String, color: Color = TEXT_COLOR) -> void:
    if _line_count:
        newline()
    push_color(color)
    add_text(line)
    pop()
    _line_count += 1
    if _line_count > MAX_LINES:
        remove_paragraph(0)
        _line_count -= 1
