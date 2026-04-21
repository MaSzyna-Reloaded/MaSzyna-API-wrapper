extends MaszynaGutTest

var console: Node


func before_each() -> void:
    console = load("res://addons/libmaszyna/console/console.gd").new()
    add_child_autoqfree(console)
    await wait_idle_frames(2)


func test_line_edit_keeps_editing_on_submit() -> void:
    assert_true(console.line_edit.keep_editing_on_text_submit, "Console input should keep editing after text submit")


func test_submit_keeps_focus_and_editing_state() -> void:
    console.set_visible(true)
    await wait_idle_frames(1)

    assert_true(console.is_visible(), "Console should be visible after opening")
    assert_eq(get_viewport().gui_get_focus_owner(), console.line_edit, "Console input should own focus when opened")
    assert_true(console.line_edit.is_editing(), "Console input should enter editing mode when opened")

    console.line_edit.text = "help"
    console.on_text_entered("help")
    await wait_idle_frames(2)

    assert_true(console.is_visible(), "Console should stay visible after command submission")
    assert_eq(get_viewport().gui_get_focus_owner(), console.line_edit, "Console input should keep focus after command submission")
    assert_true(console.line_edit.is_editing(), "Console input should stay in editing mode after command submission")
    assert_eq(console.line_edit.text, "", "Console input should be cleared after command submission")
