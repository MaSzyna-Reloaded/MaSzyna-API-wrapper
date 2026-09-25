extends MaszynaGutTest

## MaszynaLocale reads the game's .po catalogue and translates the original's msgids; the cab
## control captions go through it.

const FIXTURE_PO:String = "res://tests/fixtures/lang/pl.po"


func before_all() -> void:
    MaszynaLocale.load_translation(FIXTURE_PO)


func after_all() -> void:
    MaszynaLocale.load_translation("")


func test_translates_a_known_msgid() -> void:
    assert_eq(MaszynaLocale.gettext("master controller"), "nastawnik jazdy")


func test_unknown_msgid_stays_as_it_is() -> void:
    assert_eq(MaszynaLocale.gettext("no such string"), "no such string")


func test_missing_catalogue_leaves_every_msgid_untranslated() -> void:
    MaszynaLocale.load_translation("res://tests/fixtures/lang/missing.po")
    assert_eq(MaszynaLocale.gettext("master controller"), "master controller")
    MaszynaLocale.load_translation(FIXTURE_PO)


func test_cab_control_caption_is_translated() -> void:
    assert_eq(MmdCabControlCaptions.caption(&"mainctrl"), "nastawnik jazdy")
    assert_eq(MmdCabControlCaptions.caption(&"battery_sw"), "bateria")


func test_label_without_caption_has_none() -> void:
    assert_eq(MmdCabControlCaptions.caption(&"no_such_control"), "")


func test_wrapper_strings_come_through_gettext() -> void:
    var previous:String = TranslationServer.get_locale()
    TranslationServer.set_locale("pl")
    # not in the game's catalogue (the fixture), so from the wrapper's own
    assert_eq(MaszynaLocale.gettext("forward"), "przód")
    assert_eq(MaszynaLocale.gettext("cutoff"), "odcięcie")
    # the game's catalogue first
    assert_eq(MaszynaLocale.gettext("master controller"), "nastawnik jazdy")
    TranslationServer.set_locale(previous)
