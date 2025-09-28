#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/string.hpp>
#include <vector>

namespace godot {

class ActionQueue : public RefCounted {
        GDCLASS(ActionQueue, RefCounted)
    static const char *ACTION_REQUESTED;
    static const char *ACTION_FINISHED;
    static const char *QUEUE_FINISHED;
    static const char *QUEUE_ERROR;
    public:
        struct QueueItem {
            String title;
            Callable callable;

            QueueItem() = default;
            QueueItem(const Callable &p_callable, const String &p_title) : title(p_title), callable(p_callable) {}
        };

    private:
        std::vector<QueueItem> _queue;
        int _loadedItems = 0;
        bool _running = false;

    protected:
        static void _bind_methods();

    public:
        int get_finished_items() const;

        // Adds a new action to the queue.
        void add_item(const Callable &p_callable, const String &p_title);

        // Clears the queue and stops any current processing state.
        void clear();

        // Starts processing the queue synchronously; emits signals on finish or error.
        void start();

        // Helpers
        int64_t get_queue_size() const;
        bool is_running() const;
};

} // namespace godot
