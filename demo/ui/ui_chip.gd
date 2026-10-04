class_name UIChip
extends PanelContainer

## A floating chip of the HUD - a tile over the game (the driving aid, the simulation speed, the
## streaming spinner, a vehicle, the frame times): its panel is the chip's own look, laid on it by
## the chip itself, so the theme its content reads (a container's) stays as it is. CARD is a
## framed card - lit up under the mouse when the chip is clicked (set_hovered()) - and TEXT the
## text alone on a faint background darker where the scene behind is bright.

enum Look {
    CARD,
    TEXT,
}

const CARD_STYLE: StyleBox = preload("ui_chip_card.tres")
const HOVER_STYLE: StyleBox = preload("ui_chip_hover.tres")
const TEXT_STYLE: StyleBox = preload("ui_chip_text.tres")
const CONTRAST_SHADER: Shader = preload("ui_chip_contrast.gdshader")

@export var look: Look = Look.CARD

var _hovered: bool = false
## TEXT's background, darker where the scene behind is bright - the chip's own material, made once
var _contrast: ShaderMaterial = null


func _ready() -> void:
    _apply_look()


func set_look(p_look: Look) -> void:
    look = p_look
    _apply_look()


## The mouse is over a chip that can be clicked
func set_hovered(p_hovered: bool) -> void:
    _hovered = p_hovered
    _apply_look()


func _apply_look() -> void:
    if look == Look.TEXT:
        add_theme_stylebox_override(&"panel", TEXT_STYLE)
        if not _contrast:
            _contrast = ShaderMaterial.new()
            _contrast.shader = CONTRAST_SHADER
        material = _contrast
        return
    if _contrast and material == _contrast:
        material = null
    add_theme_stylebox_override(&"panel", HOVER_STYLE if _hovered else CARD_STYLE)
