extends MarginContainer
class_name LanguageSwitcher

## The language of the game's strings (MaszynaRuntime.language): one flag for every catalogue
## MaszynaTranslationServer finds in the game's lang/, English alone when there is none. Its left
## margin keeps it clear of what stands before it in a row.

## How large a flag is drawn
const FLAG_SIZE:Vector2 = Vector2(36.0, 24.0)
## What each language calls itself, shown on hover; a language not listed shows its code
const LANGUAGE_NAMES:Dictionary[String, String] = {
    "en": "English",
    "pl": "Polski",
    "cz": "Čeština",
    "hu": "Magyar",
    "zh": "中文",
}

## A flag per language, keyed as the catalogue is named (lang/cz.po -> "cz"); a language with no
## flag shows its code instead
@export var flags:Dictionary[String, Texture2D] = {}


func _ready() -> void:
    var group:ButtonGroup = ButtonGroup.new()
    for language:String in MaszynaTranslationServer.get_languages():
        var button:Button = Button.new()
        button.toggle_mode = true
        button.button_group = group
        button.focus_mode = Control.FOCUS_NONE
        button.tooltip_text = LANGUAGE_NAMES.get(language, language)
        if flags.has(language):
            button.icon = flags[language]
            button.expand_icon = true
            button.custom_minimum_size = FLAG_SIZE
        else:
            button.text = language.to_upper()
        button.button_pressed = language == MaszynaRuntime.language
        button.pressed.connect(_on_language_pressed.bind(language))
        %Flags.add_child(button)


func _on_language_pressed(language:String) -> void:
    MaszynaRuntime.language = language
