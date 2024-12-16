# SceneryResourceLoader singleton

@tool
extends Node

signal scenery_loaded

signal loading_started()
signal loading_finished()
signal loading_request()
signal loading_error(error)

var current_task:String = ""
var files_to_load:int = 0
var files_loaded:int = 0
var enabled:bool = true

var _queue = []
var _t:float = 0.0
var _immediate:bool = false
var _busy:bool = false
const _wait_time = 0.01

func reset():
    files_to_load = 0
    files_loaded = 0
    _queue.clear()

func schedule(title:String, callable: Callable):
    _queue.append([title, callable])
    files_to_load += 1
    loading_request.emit()

func _process(delta):
    if not enabled:
        return

    _t += delta
    if _busy or _t > _wait_time:
        _t = 0.0
        if files_loaded < files_to_load:
            _busy = true
            var x = _queue.pop_front()
            if x:
                current_task = x[0]
                loading_started.emit()
                x[1].call()
                files_loaded += 1
                loading_finished.emit()
            else:
                files_loaded = files_to_load
                loading_error.emit("Not all files were loaded. Check log.")
                _busy = false

        if _busy and files_loaded == files_to_load:
            _busy = false
            scenery_loaded.emit()
