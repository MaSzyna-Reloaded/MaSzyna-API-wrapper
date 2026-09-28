extends PanelContainer

## The scenario being played, to read again after the selector is gone: the scenery title, its
## image and description from the .scn header, and the mission of the trainset the player chose
## (the same header the scenario selector shows, MaszynaSceneryInfo).

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


func _on_close_button_pressed() -> void:
    close_requested.emit()
