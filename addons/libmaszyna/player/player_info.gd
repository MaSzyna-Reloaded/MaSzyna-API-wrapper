extends Resource
class_name PlayerInfo

enum PlayerType { PLAYER_TYPE_LOCAL, PLAYER_TYPE_REMOTE }

@export var peer_id:int
@export var name:String
@export var type:PlayerType
@export var train_id:String
