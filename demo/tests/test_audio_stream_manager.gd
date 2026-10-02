extends MaszynaGutTest

## A sound's length read off its Ogg pages, before the file is ever loaded to play

const FIXTURES_GAME_DIR:String = "res://tests/fixtures"
const SOUND:String = "test_loop"
const MISSING_SOUND:String = "no_such_sound"
const LENGTH_TOLERANCE:float = 0.001

var _previous_game_dir:String = ""


func before_each() -> void:
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    UserSettings.save_maszyna_game_dir(FIXTURES_GAME_DIR)


func after_each() -> void:
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func test_length_read_off_the_pages_is_the_decoded_length() -> void:
    var decoded:AudioStream = AudioStreamManager.get_stream(SOUND)

    assert_gt(decoded.get_length(), 0.0)
    assert_almost_eq(AudioStreamManager.get_stream_length(SOUND), decoded.get_length(), LENGTH_TOLERANCE)


func test_a_missing_sound_has_no_length() -> void:
    assert_eq(AudioStreamManager.get_stream_length(MISSING_SOUND), 0.0)
