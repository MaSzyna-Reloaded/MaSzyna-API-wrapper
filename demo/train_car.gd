extends Object

class_name TrainCarGD

const FRONT = "front"
const BACK = "back"

var id
var front = null
var back = null

var trainset:TrainSetGD

func _init(_id: String):
    id = _id
    trainset = TrainSetGD.new(self)

func _to_string() -> String:
    return "TrainCar(%s)" % id

func couple(other_car: TrainCarGD, self_side: String, other_side: String) -> void:
    if self_side == FRONT and other_side == BACK:
        if front or other_car.back:
            push_error("One of the cars is already coupled.")
            return
        front = other_car
        other_car.back = self
    elif self_side == BACK and other_side == FRONT:
        if back or other_car.front:
            push_error("One of the cars is already coupled.")
            return
        back = other_car
        other_car.front = self
    else:
        push_error("Invalid coupling sides specified.")

func couple_front(other_car: TrainCarGD, other_side: String) -> void:
    couple(other_car, FRONT, other_side)

func couple_back(other_car: TrainCarGD, other_side: String) -> void:
    couple(other_car, BACK, other_side)

func uncouple_front() -> TrainCarGD:
    if not front:
        push_error("No car coupled at the front.")
        return null
    var uncoupled = front
    front.back = null
    front = null
    return uncoupled

func uncouple_back() -> TrainCarGD:
    if not back:
        push_error("No car coupled at the back.")
        return null
    var uncoupled = back
    back.front = null
    back = null
    return uncoupled
