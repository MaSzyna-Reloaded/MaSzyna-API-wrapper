extends Resource
class_name E3DSubModel

enum SubModelType {
    GL_POINTS=0,
    GL_LINES,
    GL_LINE_STRIP,
    GL_LINE_LOOP,
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_TRIANGLE_FAN,
    GL_QUADS,
    GL_QUAD_STRIP,
    GL_POLYGON,
    TRANSFORM = 256,
    FREE_SPOTLIGHT,
    STARS,
}

enum AnimationType {
    NONE=0,
    ROTATE_VEC=1,
    ROTATE_XYZ,
    MOVE,
    JUMP_SECONDS,
    JUMP_MINUTES,
    JUMP_HOURS,
    JUMP_HOURS24,
    SECONDS,
    MINUTES,
    HOURS,
    HOURS24,
    BILLBOARD,
    WIND,
    SKY,
    DIGITAL,
    DIGICLK,
    UNDEFINED,
    IK=256,
    IK1=257,
    IK2=258,
    UNKOWN=-1
}

@export var name:String = ""
@export var submodel_type:SubModelType = SubModelType.GL_TRIANGLES
@export var animation = AnimationType.NONE
@export_range(-1.0, 1.0, 0.01) var lights_on_threshold:float = 0.0
@export_range(-1.0, 1.0, 0.01) var visibility_light:float = 0.0
@export var visibility_range_begin:float = 0.0
@export var visibility_range_end:float = 0.0
@export var diffuse_color:Color = Color.WHITE
@export var self_illumination:Color = Color.WHITE
@export var material_colored:bool = false
@export var dynamic_material:bool = false
@export var dynamic_material_index:int = 0
@export var material_transparent:bool = false
@export var material_name:String = ""
@export var transform:Transform3D = Transform3D()
@export var mesh:ArrayMesh
@export var submodels:Array[E3DSubModel] = []
@export var visible:bool = true
@export var skip_rendering:bool = false

var parent:E3DSubModel

func add_child(submodel:E3DSubModel):
    submodels.append(submodel)

func set_parent(submodel:E3DSubModel):
    parent = submodel
    if submodel:
        submodel.add_child(self)
