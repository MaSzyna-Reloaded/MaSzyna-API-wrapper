extends Node2D

func _ready():
    var engine = RailVehicle.new()
    engine.name = "Engine"
    var passenger1 = RailVehicle.new()
    var passenger2 = RailVehicle.new()

    engine.couple_back(passenger1, RailVehicle.Side.FRONT)
    passenger1.couple_back(passenger2, RailVehicle.Side.FRONT)

    print("Original Train: ", engine.get_trainset().to_array())

    var new_train = passenger1.uncouple_back()
    print("After Split:")
    print("Front Train: ", engine.get_trainset().to_array())

    var passenger3 = RailVehicle.new()
    engine.get_trainset().get_tail().couple_back(passenger3, RailVehicle.FRONT)

    print("Passenger3's Train: ", passenger3.get_trainset().to_array())
    print("Second car of trainset: ", engine.get_trainset().get_by_index(1))
    print("Passenger2's trainset: ", passenger2.get_trainset().to_array())
