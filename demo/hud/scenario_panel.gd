extends PanelContainer

## The scenario being played, to read again after the selector is gone: the scenery title, its
## image and description from the .scn header, and the mission of the trainset the player chose
## (the same header the scenario selector shows, MaszynaSceneryInfo). Its second tab tells what the
## player is to do in the vehicle they drive (ScenarioTask), read on a Timer while that tab is
## shown; the vehicle is the player's of the moment (PlayerServer), kept nowhere here.

## The close button asks the owner of the View menu to hide the panel and untick its entry
signal close_requested


## The scenario the player has started; an empty train_id lets the scenery pick its own driver, so
## there is no chosen trainset whose mission could be shown
func show_scenario(info:MaszynaSceneryInfo, train_id:String) -> void:
    %Title.text = info.title
    %Description.text = info.description
    %Description.visible = not info.description == ""
    %Image.texture = null
    if info.image_path:
        var image:Image = Image.load_from_file(info.image_path)
        if image:
            %Image.texture = ImageTexture.create_from_image(image)
    %Image.visible = not %Image.texture == null
    %Mission.text = ""
    for trainset:MaszynaSceneryInfo.Trainset in info.trainsets:
        if train_id and trainset.get_driver_train_id() == train_id:
            %Mission.text = trainset.description
            break
    %MissionSection.visible = not %Mission.text == ""


func _on_progress_visibility_changed() -> void:
    if %"Scenario progress".is_visible_in_tree():
        %RefreshTimer.start()
        _on_refresh_timer_timeout()
    else:
        %RefreshTimer.stop()


func _on_refresh_timer_timeout() -> void:
    var task:ScenarioTask = ScenarioTask.describe(PlayerServer.player_get_vehicle())
    %ProgressTitle.text = task.title
    %ProgressText.text = "\n\n".join(task.paragraphs)
    %ProgressText.visible = not %ProgressText.text == ""


func _on_close_button_pressed() -> void:
    close_requested.emit()
