#include "ActionQueue.hpp"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    const char *ActionQueue::ACTION_REQUESTED = "action_requested";
    const char *ActionQueue::ACTION_FINISHED = "action_finished";
    const char *ActionQueue::QUEUE_FINISHED = "queue_finished";
    const char *ActionQueue::QUEUE_ERROR = "queue_error";

    void ActionQueue::_bind_methods() {
        ClassDB::bind_method(D_METHOD("add_item", "callable", "title"), &ActionQueue::add_item);
        ClassDB::bind_method(D_METHOD("get_finished_items"), &ActionQueue::get_finished_items);
        ClassDB::bind_method(D_METHOD("clear"), &ActionQueue::clear);
        ClassDB::bind_method(D_METHOD("start"), &ActionQueue::start);
        ClassDB::bind_method(D_METHOD("get_queue_size"), &ActionQueue::get_queue_size);
        ClassDB::bind_method(D_METHOD("is_running"), &ActionQueue::is_running);

        ADD_SIGNAL(MethodInfo(ACTION_REQUESTED));
        ADD_SIGNAL(MethodInfo(ACTION_FINISHED));
        ADD_SIGNAL(MethodInfo(QUEUE_FINISHED));
        ADD_SIGNAL(MethodInfo(QUEUE_ERROR, PropertyInfo(Variant::STRING, "error"), PropertyInfo(Variant::INT, "index"),
                PropertyInfo(Variant::STRING, "title")));
    }

    int ActionQueue::get_finished_items() const {
        return _loadedItems;
    }

    void ActionQueue::add_item(const Callable &p_callable, const String &p_title) {
        _queue.emplace_back(p_callable, p_title);
    }

    void ActionQueue::clear() {
        _queue.clear();
        _running = false;
    }

    void ActionQueue::start() {
        if (_running) {
            // Already running, ignore subsequent starts.
            return;
        }
        _running = true;

        for (size_t i = 0; i < _queue.size(); ++i) {
            const QueueItem &item = _queue[i];
            if (!item.callable.is_valid()) {
                emit_signal(QUEUE_ERROR, Variant("Invalid callable"), Variant(static_cast<int64_t>(i)), item.title);
                _running = false;
                return;
            }

            // Call with no arguments as per requirement.
            // Using callv to avoid template instantiation confusion; empty arguments list.
            const Array args;
            if (Variant ret = item.callable.callv(args); ret.get_type() != Variant::NIL) {
                emit_signal(ACTION_FINISHED, Variant(static_cast<int64_t>(i)), item.title);
                _loadedItems++;
            }
        }

        _running = false;
        emit_signal(QUEUE_FINISHED);
    }

    int64_t ActionQueue::get_queue_size() const { return static_cast<int64_t>(_queue.size()); }

    bool ActionQueue::is_running() const { return _running; }

} // namespace godot
