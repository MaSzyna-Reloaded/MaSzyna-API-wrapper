@tool
extends Container
class_name DebugPanel

var _dirty = false

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
        $Title.text = title
        $DebugPanel/RichTextLabel.text = text
