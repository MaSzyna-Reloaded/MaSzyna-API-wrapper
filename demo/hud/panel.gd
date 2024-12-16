@tool
extends Container
class_name DebugPanel

var _dirty = false
var _filter:String = ""

@export var text:String:
    set(x):
        _dirty = true
        text = x

@export var title:String:
    set(x):
        _dirty = true
        title = x


func _process(_delta):
    if _dirty:
        _dirty = false
        $%Title.text = title
        var final_text = text
        if _filter:
            var _tokens = _filter.split(" ")
            var _f = []
            for line in text.split("\n"):
                var _matched_count = 0
                for _t in _tokens:
                    if not _t or line.match("*"+_t+"*"):
                        _matched_count += 1
                if _tokens.size() == _matched_count:
                    _f.append(line)

            final_text = "\n".join(_f)
        $DebugPanel/RichTextLabel.text = final_text


func _on_filter_text_changed(new_text):
    _filter = new_text


func _on_clear_button_up():
    _filter = ""
    $%Filter.text = ""
