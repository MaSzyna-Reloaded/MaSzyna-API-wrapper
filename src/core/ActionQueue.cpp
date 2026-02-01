#include "ActionQueue.hpp"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    const char *ActionQueue::LOADING_REQUEST = "loading_request";
    const char *ActionQueue::LOADING_STARTED = "loading_started";
    const char *ActionQueue::LOADING_FINISHED = "loading_finished";
    const char *ActionQueue::SCENERY_LOADED = "scenery_loaded";
    const char *ActionQueue::LOADING_ERROR = "loading_error";

    ActionQueue::ActionQueue() {
        set_process(true);
    }

    void ActionQueue::_bind_methods() {
        ClassDB::bind_method(D_METHOD("schedule", "title", "callable"), &ActionQueue::schedule);
        ClassDB::bind_method(D_METHOD("reset"), &ActionQueue::reset);

        // Properties
        ClassDB::bind_method(D_METHOD("set_files_to_load", "files_to_load"), &ActionQueue::set_files_to_load);
        ClassDB::bind_method(D_METHOD("get_files_to_load"), &ActionQueue::get_files_to_load);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "files_to_load"), "set_files_to_load", "get_files_to_load");

        ClassDB::bind_method(D_METHOD("set_files_loaded", "files_loaded"), &ActionQueue::set_files_loaded);
        ClassDB::bind_method(D_METHOD("get_files_loaded"), &ActionQueue::get_files_loaded);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "files_loaded"), "set_files_loaded", "get_files_loaded");

        ClassDB::bind_method(D_METHOD("set_current_task", "current_task"), &ActionQueue::set_current_task);
        ClassDB::bind_method(D_METHOD("get_current_task"), &ActionQueue::get_current_task);
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "current_task"), "set_current_task", "get_current_task");

        ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &ActionQueue::set_enabled);
        ClassDB::bind_method(D_METHOD("get_enabled"), &ActionQueue::get_enabled);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");

        // Old methods compatibility/aliases if needed
        ClassDB::bind_method(D_METHOD("add_item", "callable", "title"), &ActionQueue::add_item);

        ADD_SIGNAL(MethodInfo(LOADING_REQUEST));
        ADD_SIGNAL(MethodInfo(LOADING_STARTED));
        ADD_SIGNAL(MethodInfo(LOADING_FINISHED));
        ADD_SIGNAL(MethodInfo(SCENERY_LOADED));
        ADD_SIGNAL(MethodInfo(LOADING_ERROR, PropertyInfo(Variant::STRING, "error")));
    }

    int ActionQueue::get_files_to_load() const { return files_to_load; }
    void ActionQueue::set_files_to_load(int p_value) { files_to_load = p_value; }

    int ActionQueue::get_files_loaded() const { return files_loaded; }
    void ActionQueue::set_files_loaded(int p_value) { files_loaded = p_value; }

    String ActionQueue::get_current_task() const { return current_task; }
    void ActionQueue::set_current_task(const String &p_value) { current_task = p_value; }

    bool ActionQueue::get_enabled() const { return enabled; }
    void ActionQueue::set_enabled(bool p_value) { enabled = p_value; }

    void ActionQueue::reset() {
        files_to_load = 0;
        files_loaded = 0;
        _queue.clear();
        current_task = "";
        _busy = false;
        _t = 0.0;
    }

    void ActionQueue::schedule(const String &p_title, const Callable &p_callable) {
        _queue.emplace_back(p_callable, p_title);
        files_to_load++;
        emit_signal(LOADING_REQUEST);
    }

    void ActionQueue::add_item(const Callable &p_callable, const String &p_title) {
        schedule(p_title, p_callable);
    }

    void ActionQueue::_process(double delta) {
        if (!enabled) {
            return;
        }

        _t += delta;
        if (_busy || _t > _wait_time) {
            _t = 0.0;
            if (files_loaded < files_to_load) {
                _busy = true;
                if (!_queue.empty()) {
                    QueueItem item = _queue.front();
                    _queue.pop_front();

                    current_task = item.title;
                    emit_signal(LOADING_STARTED);

                    if (item.callable.is_valid()) {
                        item.callable.call();
                    } else {
                        emit_signal(LOADING_ERROR, "Invalid callable");
                    }

                    files_loaded++;
                    emit_signal(LOADING_FINISHED);
                } else {
                    // Queue empty but files_loaded < files_to_load?
                    // This means some logic error or we should sync them.
                    // For now, let's assume if queue is empty we are done or something went wrong.
                    // But if we follow SceneryResourceLoader logic:
                    // if x: ... else: files_loaded = files_to_load; loading_error...

                    files_loaded = files_to_load;
                    emit_signal(LOADING_ERROR, "Not all files were loaded. Check log.");
                    _busy = false;
                }
            }

            if (_busy && files_loaded == files_to_load) {
                _busy = false;
                emit_signal(SCENERY_LOADED);
            }
        }
    }

} // namespace godot
