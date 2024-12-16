@tool
extends Node

# This is a singleton class which will contain the current state of the simulation environment

enum TimeOfDay {
    DAY = 0,
    NIGHT
}

signal timeofday_changed(timeofday: TimeOfDay)
