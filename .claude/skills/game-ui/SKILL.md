---
name: game-ui
description: Design and build player-facing UI in demo/ - screens, side panels, settings pages, popups and confirmation questions, buttons, rows and their disabled/focused states, labels and their translations. Use before adding or changing a screen, a dialog or popup, a button row, a settings entry (settings.json), a control's look or a user-visible text, and when the operator says the UI is inconsistent, jumps, looks like default Godot, or a key does not work in it.
---

# Game UI - consistent, keyboard-complete, translated

The operator judges UI by one thing first: **does it look and behave like the rest of the game.**
A new element that is correct on its own but differs from its neighbour - another button style,
another order, Godot's grey dialog, a row that changes height - is a defect, not a detail. Every
rule below was paid for in a session where it went wrong.

## 1. Read the request literally, scope included

* A rule given for named sections applies to **those sections only**. "Graphics stays, Scenery
  keeps X and Y, the rest is advanced" means the rest *of Scenery* - not every other section of
  the file. Extending it hid ten sections and was the costliest mistake of the session.
* When a sentence has two readings that lead to different code (e.g. "either TAA or MSAA and
  FXAA": one combined option, or three options that exclude each other), ask with
  `AskUserQuestion` before writing anything. When it has one reading, do not ask.
* "Similar style and order" means the same look and the same placement - **not** a shared
  abstraction. Do not refactor styles into a theme or a common class unless asked; copying the
  scene's own style resources into the new scene is the expected way.

## 2. Find the existing element of the same meaning, then copy it

Before building anything, grep `demo/**/*.tscn` for the element that already does the same job,
and take its look, size, order and keys:

| Need | Use |
|---|---|
| A window (a screen's frame, a form) | `ui/ui_window.tscn` (`UIWindow`, a borderless Godot `Window` that keeps its keys): title and × at the top, the user's nodes under `Frame/Layout/Body` (editable children); `close_requested` from × and from an Escape nothing inside took; `show_part(ratio)` shows it. It covers the whole game window with its own `Backdrop` (dim; cleared where the game must stay visible) and the frame anchored in the middle, so a click beside the frame stays in it - never `exclusive`: Godot drops the window manager's close of the game while an exclusive child is shown. Settings screen, problem report |
| Any popup over a screen - a notice, a question, Back / Discard / Save | `ui/ui_dialog.tscn` (`UIDialog`, `top_level`): `title`, `message`, `confirm_text`, `alternative_text`, `back_text` (empty hides the button), `confirm_intent` / `alternative_intent`; `ask()`; signals `confirmed`, `alternative_chosen`, `dismissed`. Settings, development notice, exit to menu, remove trainset, report sent |
| A button | `UIButton` (`ui/ui_button.gd` on a `Button`): `intent` `DEFAULT` / `SUCCESS` green / `WARNING` orange / `DANGER` reddish; it styles itself, takes no Godot focus |
| A row of buttons a keyboard walks | `UIActionButtons` (`ui/ui_action_buttons.gd` on an `HBoxContainer`): its own focus like a `FocusSection` (`grab_button_focus()` / `release_button_focus()`), arrows and Tab choose (`tab_walks_buttons` off where Tab walks sections), Enter presses; the chosen button is lit and framed white |
| On / off | `UISwitch` (`ui/ui_switch.gd` on a `CheckButton`) - never a `CheckBox`, never a bare `CheckButton` (the old HUD windows keep their own) |
| A value in a range | `UISlider` (`ui/ui_slider.gd` on an `HSlider`); in a settings row its value is a `UITextField` to type it into |
| Something below the main things (the settings' "Advanced") | the controls' `set_compact(true)` (`UISlider`, `UISwitch`, `UISelect`, `UITextField`): smaller font and icons; the advanced row itself washed out, square and frameless (`SettingItemAdvanced*`; editing shown by the row's own blue lit up - never another hue) and a slider on one line with its name |
| A text or a number the player types | `UITextField` (`ui/ui_text_field.gd` on a `LineEdit`), framed like the search field; the only control that takes Godot's focus - a section that walks keys leaves them alone while a field has it |
| One of a few named values | `UISelect` (`ui/ui_select.gd` on an `OptionButton`; its list takes the look from it) |
| A floating tile over the game | `UIChip` (`ui/ui_chip.gd` on a `PanelContainer`, or a chip's own script `extends UIChip` calling `super()` in `_ready()`): `CARD` framed (`set_hovered()` lights it under the mouse), `TEXT` faint background with contrast (`DrivingAid.apply_style()` picks it from `hud/driving_aid/style`). It lays only its panel style on itself, so the theme its content reads stays. Driving aid tiles, simulation speed, streaming spinner, vehicle chips, the F3 frame times |
| A keyboard-navigable block | `ui/focus_section.gd` (`FocusSection`) |

Each control component loads `ui/ui_theme.tres` itself (a container component - `UIChip` - lays only its own
style, never a theme, or it would take the theme its content reads), so it looks the same wherever it stands; no screen
theme carries a control's look. Its assets live in `demo/ui/` next to it.

* **A mechanism used twice is a component, never carved by hand into each screen.** When the
  game needs something the existing components do not do - a dialog, a list, a grid - build it
  once in `demo/ui/` as a scene with a `class_name` script (as `UIDialog`, `FocusSection`,
  `SelectorList`, `TileGrid`), configured by `@export`s and answering by signals, and instance
  it where it is needed. It names nothing of its consumers.
* **A new generic component replaces every older way of doing its job, in the same work** -
  the hard rule of `AGENTS.md` ("whoever introduces something generic..."). Introducing
  `UIDialog` meant grepping `demo/` for every popup - the hand-made development notice, every
  `AcceptDialog` / `ConfirmationDialog` - and moving each onto it, deleting the old scene. A
  generic thing next to the hand-made things it was meant to replace is two roads to one goal:
  the operator meets the old one first ("the startup popup takes neither Enter nor Escape").
* **Never change a default or a generic to restyle the game.** No project-wide theme
  (`gui/theme/custom`), no base-type entry meant for every control: Godot's defaults reach the
  debug HUD (`ControlWindows`, `hud/window.gd`), the addons (console, F3) and anything else with a
  look of its own, and change them all. Setting `ui_theme.tres` as the project theme to give the
  game one font and one tooltip changed the debug HUD's font and tooltips; it was taken back. The
  game's look goes into the game's own themes - `ui_theme.tres` (components, windows, dialogs),
  `selector_theme.tres` (selector, settings), `timetable_theme.tres` (HUD cards, chips, menus),
  the loading board's - each pointing at the one shared resource (`ui/fonts/*.tres`,
  `ui/ui_tooltip.tres`) rather than holding a copy.
* **Moving onto a component never restyles a place that has a look of its own.** The old green
  HUD windows (`hud/window.gd`, the weather and time controls, `hud/switch.tscn`) have their own
  style; putting `UISlider` / `UISwitch` into them made half a window blue beside the green rest,
  and it was taken back. Such a place is named to the operator and left until they decide - the
  rule above moves what does the same job *in the same look*.
* **Never use Godot's `AcceptDialog` / `ConfirmationDialog` in the game UI.** It draws Godot's
  grey window unless themed, its OK/Cancel order follows the platform (`swap_cancel_ok`), its
  buttons are not the screen's buttons, and its Escape goes through `ui_close_dialog` of a focused
  sub-window - which the screen's own `_input` can swallow. A dialog is a `UIDialog`.
* **Buttons of a popup are the screen's buttons**: the same styles, sizes and font, and the same
  order - the alternative on the left, the answer on the right. "Discard changes" left of "Save"
  in the footer means "Discard changes" left of "Save" in the question too.
* A button has the colour of what it does, **wherever it stands**: green for one that keeps
  ("Save", `UIButton.Intent.SUCCESS`), reddish for one that throws away or leaves ("Discard
  changes", "Remove", "Exit to menu", `DANGER`), plain blue-outlined for the rest. "Discard
  changes" is reddish as the answer of one dialog and as the alternative of another.
* A question that can be left without an answer has "Back" (`back_text`) at the left, doing what
  Escape does - "Back / Discard changes / Save", not just "Discard changes / Save".
* A destructive action stands apart from the confirming ones: "Reset to defaults" at the left
  edge of the button row (`size_flags_horizontal = 2` on it pushes the rest right - no spacer node).

## 3. A popup is complete only with its keys

* While shown it takes **every** key (`_input`, `set_input_as_handled()` first) and the backdrop
  stops the mouse, so nothing under it reacts.
* Escape (`menu_back`) always takes it back unanswered. Enter (`menu_activate`) takes the chosen
  button. Both ways of walking buttons the player tries work: the arrows (`menu_left` /
  `menu_right`) **and** Tab / Shift+Tab (`menu_next_section` / `menu_previous_section`). All
  project actions, exact match - see `AGENTS.md`. A key the player presses and nothing happens is
  a defect even if another key does the job.
* The chosen button must be plain to see: its own colour lit as under the mouse **and** a white
  2 px frame (`UIButton*Chosen` variations in `ui_theme.tres`). A thin grey outline, and a lit
  background alone, were both rejected as not telling which button is chosen. The answer is
  chosen when the dialog opens.
* **The game's keyboard focus is virtual** (`FocusSection`, `UIActionButtons`): a screen keeps
  Godot's focus in its search field, so a button that relies on Godot's focus is never chosen
  there. Buttons take no Godot focus at all.
* Never read a theme item (`get_theme_stylebox`) to build a look at run time: in the frame a
  control gets its theme the cache is still the default one, and the look comes out grey. A look
  is a theme variation the control switches to.
* The window's close button (×) asks what to do with the changes - "Save" or "Discard changes" -
  it does not silently cancel. "Discard changes" asks for confirmation.

## 4. States must not move the layout

* A disabled control must keep its size. Godot falls back to its **default** theme items for every
  `*_disabled` state the project theme does not define - other icons, other margins - and the row
  jumps. When a control can be disabled, give the theme the disabled items with the same
  resources as the enabled ones: `CheckButton/icons/checked_disabled`, `unchecked_disabled`,
  `CheckButton/styles/disabled`, `OptionButton/styles/disabled`, `HSlider/icons/grabber_disabled`.
  Show "disabled" by dimming (`modulate.a`), not by another resource.
* Settings that depend on or exclude each other are **greyed out**, not changed behind the
  player's back: `depends_on` (available only while the other is on) and `excludes` (not
  switchable on while the other is on). A row that is on is always switchable off, so a
  combination left in an old `settings.cfg` cannot lock the player out.

## 5. Sizes are proportions, not pixels

The UI is scalable and adaptive: one scene stands in the wide settings window, in the panel a
third of the screen wide beside the game, at any resolution. **Do not position or size things in
absolute pixels.**

* Layout comes from containers: `size_flags` expand/fill and `size_flags_stretch_ratio` share a
  line in a proportion (an advanced settings row: name 2, slider 3), anchors place a window or a
  panel as a part of the screen (`popup_centered_ratio()`, anchors 0.12-0.88), margins and
  separations are the containers' own.
* A column that must line up across rows (a value field, a unit) is aligned by every row having
  the same structure - the same containers, the same shares, an empty unit column where there is
  no unit - not by a pixel offset.
* A fixed pixel width was rejected twice in one session: a slider of 140 px was too narrow to be
  precise, and one of 420 px left the narrow panel no room for the names, which vanished. A share
  of the line fits both.
* Where a size cannot be a proportion - a control's minimum so it is usable (a button's height, a
  field's minimum width for its digits), an icon - it is the smallest that works and is named as a
  constant or set in the scene, never a layout decided in pixels. A name that does not fit is cut
  with an ellipsis and shown whole in a tooltip, it does not push the layout.
* Check a layout at more than one size: the settings window and the in-game panel, and a probe
  with the window resized (a headless window is 64 px - measure with `Window.size` set).

## 6. Words

* A label says the effect: "Discard changes", not "Cancel" or "Revert changes"; "Reset to
  defaults", not "Default settings"; a popup button repeats the full phrase, not "Discard".
* A toggle that shares its name with its section gets its own: the section "Smoke" holds
  "Smoke enabled".
* Every new user-visible string is a msgid in `demo/translations/template.po` **and** in every
  language file (`pl`, `cz`, `hu`, `zh`) in the same change; a msgid nothing uses any more is
  removed from all of them. Do not leave translations for later - the operator asks for them.
  Translations other than Polish are proposals; say so in the report.

## 7. Settings screen data (`demo/settings/settings.json`)

* The order in the file is the order on the screen - the operator decides it; most noticeable
  first (V-Sync, then anti-aliasing, then the screen-space effects).
* `"advanced": true` keeps an entry off the screen; a section with nothing left is not listed.
* Diagnostics and developer switches are not player settings at all: they live under
  `maszyna/debug/...` and never appear in `settings.json`, not even as advanced.

## 8. Report what was verified

Parse-checks and a short headless run prove the scripts load - they say nothing about how it looks
or whether a key works in the running game. Say exactly that in the report; never present a
layout or an input fix as working before the operator has seen it.
