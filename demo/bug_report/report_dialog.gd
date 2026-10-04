extends UIWindow

## The form of a problem report in a window of the game (UIWindow): what happened, the screenshot
## with its marks, and the consent to publish it, without which nothing is sent. Its Send and Close
## buttons and its close request (the close button, Escape) are wired to the report
## (bug_report.tscn), which decides what they do; a report sent or saved closes it, a failure stays
## on it with what was written.

## Part of the screen the form takes
const SIZE_RATIO: float = 0.85
## The screenshot's JPEG quality - a fraction of its size as a PNG, plenty to see the problem by
const SCREENSHOT_QUALITY: float = 0.75


## A new report on the screenshot: nothing written, nothing agreed to, the screenshot attached
func begin(screenshot: Image) -> void:
    %Description.text = ""
    %AttachScreenshot.button_pressed = true
    %Consent.button_pressed = false
    %Status.text = ""
    %ScreenshotAnnotator.set_image(screenshot)
    _set_form_locked(false)
    show_part(SIZE_RATIO)
    %Description.grab_focus()


func get_description() -> String:
    return %Description.text.strip_edges()


## The screenshot with its marks as JPEG, empty when it is not attached
func render_screenshot() -> PackedByteArray:
    if not %AttachScreenshot.button_pressed:
        return PackedByteArray()
    return %ScreenshotAnnotator.render_annotated_image().save_jpg_to_buffer(SCREENSHOT_QUALITY)


func show_sending() -> void:
    %Status.text = tr("Sending...")
    _set_form_locked(true)


func show_failed(message: String) -> void:
    %Status.text = tr("Sending failed: %s") % message
    _set_form_locked(false)


## While a report is being sent, nothing of it can be changed or sent again
func _set_form_locked(locked: bool) -> void:
    %Description.editable = not locked
    %AttachScreenshot.disabled = locked
    %ScreenshotAnnotator.mouse_filter = Control.MOUSE_FILTER_IGNORE if locked else Control.MOUSE_FILTER_STOP
    %Consent.disabled = locked
    _update_send_button()


## Send is open once something is written and the player has agreed to publish it (the
## description's and the consent's changes are wired here)
func _update_send_button() -> void:
    %SendButton.disabled = %Consent.disabled or not (%Consent.button_pressed and get_description())


func _on_attach_screenshot_toggled(pressed: bool) -> void:
    %ScreenshotAnnotator.visible = pressed
    %AnnotationHint.visible = pressed
