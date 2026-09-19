extends MaszynaGutTest

## SceneryLoadingTaskQueue runs tasks on worker threads; scenery includes parsed as queue tasks
## give the same result, in the same order, as parsing them in place.

const FIXTURES_GAME_DIR:String = "res://tests/fixtures"

var _previous_game_dir:String = ""


func before_each() -> void:
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    UserSettings.save_maszyna_game_dir(FIXTURES_GAME_DIR)


func after_each() -> void:
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func test_more_tasks_than_workers_return_results_by_id() -> void:
    var queue := SceneryLoadingTaskQueue.new()
    var task_count:int = queue.get_worker_count() * 4
    var task_ids:Array[int] = []
    for i:int in task_count:
        task_ids.append(queue.submit(_double.bind(i)))
    for i:int in task_count:
        assert_eq(queue.wait(task_ids[i]), i * 2)
    assert_eq(queue.get_completed_count(), task_count)


func test_nested_waiting_tasks_do_not_deadlock() -> void:
    var queue := SceneryLoadingTaskQueue.new()
    var task_id:int = queue.submit(_count_leaves.bind(queue, 6))
    assert_eq(queue.wait(task_id), 64)


func test_threaded_includes_match_in_place_parsing() -> void:
    var in_place := MaszynaImporterContext.new()
    SceneryInstancer.parse_file("threaded/root.scn", {}, in_place)

    var queue := SceneryLoadingTaskQueue.new()
    var threaded:MaszynaImporterContext = SceneryInstancer.parse_file_task(
        "threaded/root.scn", {}, MaszynaImporterContext.new().get_state(), queue
    )

    var expected:Array[String] = ["root1", "a1", "a_b", "a2", "root2", "b1", "root3"]
    assert_eq(_describe(in_place.models).map(func(entry:Array) -> String: return entry[0]), expected)
    assert_eq(_describe(threaded.models), _describe(in_place.models))
    assert_eq(threaded.dependencies.keys().size(), 3)
    assert_true(threaded.cacheable)


func _describe(models:Array[MaszynaModelData]) -> Array:
    return models.map(func(model:MaszynaModelData) -> Array: return [model.model_filename, model.position])


func _double(value:int) -> int:
    return value * 2


func _count_leaves(queue:SceneryLoadingTaskQueue, depth:int) -> int:
    if depth == 0:
        return 1
    var first:int = queue.submit(_count_leaves.bind(queue, depth - 1))
    var second:int = queue.submit(_count_leaves.bind(queue, depth - 1))
    return queue.wait(first) + queue.wait(second)
