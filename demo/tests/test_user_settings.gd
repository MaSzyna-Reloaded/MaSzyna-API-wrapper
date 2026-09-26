extends MaszynaGutTest

## Regression: the game dir was "." in an exported build. A bare relative path handed to
## FileAccess resolves against res:// - in an export the embedded pack, which holds no scenery -
## so every path built from it was unreachable, and the scenery cache's own validity check
## (SceneryInstancer._is_cache_valid()) failed on the first dependency of every entry. The result
## was a full re-parse of every scenery and subscene on each launch, while the editor, where the
## dir is a real path, cached correctly (see FINDINGS.md, 2026-09-23).
func test_game_dir_is_a_path_fileaccess_can_reach() -> void:
    # the trap itself: a relative path is res://, not the process's working directory
    assert_true(FileAccess.file_exists("project.godot"), "a bare relative path reads from res://")
    assert_true(
        UserSettings.get_maszyna_game_dir().is_absolute_path(),
        "the game dir must be absolute, or everything joined to it resolves inside res://",
    )
