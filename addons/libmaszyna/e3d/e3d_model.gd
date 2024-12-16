extends Resource
class_name E3DModel

@export var name:String = ""
@export var submodels:Array[E3DSubModel] = []

func add_child(submodel:E3DSubModel):
    submodels.append(submodel)
