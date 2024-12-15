extends Object
class_name TrainSetGD

var start_car: TrainCarGD

func _init(_start_car: TrainCarGD):
    start_car = _start_car

func get_by_index(index: int) -> TrainCarGD:
    var current
    if index >= 0:
        current = get_head()
        for x in range(index):
            if not current:
                push_error("Index out of range.")
                return null
            current = current.back
    else:
        current = get_tail()
        for x in range(-1, index, -1):
            if not current:
                push_error("Index out of range.")
                return null
            current = current.front
    if not current:
        push_error("Index out of range.")
    return current

func to_array() -> Array:
    var result = []
    var current = get_head()
    while current:
        result.append(current)
        current = current.back
    return result

func get_head() -> TrainCarGD:
    var current = start_car
    while current.front:
        current = current.front
    return current

func get_tail() -> TrainCarGD:
    var current = start_car
    while current.back:
        current = current.back
    return current
