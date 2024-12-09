@tool
extends EditorPlugin

func _enter_tree():
    add_autoload_singleton("Console", "res://addons/libmaszyna/console.gd")


func _exit_tree():
    remove_autoload_singleton("Console")
