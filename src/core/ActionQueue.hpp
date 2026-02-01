#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/string.hpp>
#include <list>

namespace godot {

class ActionQueue : public Node {
        GDCLASS(ActionQueue, Node)

    // Signals
    static const char *LOADING_REQUEST;
    static const char *LOADING_STARTED;
    static const char *LOADING_FINISHED;
    static const char *SCENERY_LOADED;
    static const char *LOADING_ERROR;

    public:
        struct QueueItem {
            String title;
            Callable callable;

            QueueItem() = default;
            QueueItem(const Callable &p_callable, const String &p_title) : title(p_title), callable(p_callable) {}
        };

    private:
        std::list<QueueItem> _queue;

        String current_task;
        int files_to_load = 0;
        int files_loaded = 0;
        bool enabled = true;

        double _t = 0.0;
        bool _busy = false;
        const double _wait_time = 0.01;

    protected:
        static void _bind_methods();

    public:
        ActionQueue();

        // Properties
        int get_files_to_load() const;
        void set_files_to_load(int p_value); // Usually read-only but property system might need setter, or we use _get/_set

        int get_files_loaded() const;
        void set_files_loaded(int p_value);

        String get_current_task() const;
        void set_current_task(const String &p_value);

        bool get_enabled() const;
        void set_enabled(bool p_value);

        // Methods
        void schedule(const String &p_title, const Callable &p_callable);
        void reset();
        void _process(double delta) override;

        // Compatibility/Helpers
        int get_finished_items() const { return files_loaded; }
        int64_t get_queue_size() const { return static_cast<int64_t>(_queue.size()); }
        void add_item(const Callable &p_callable, const String &p_title); // Alias for schedule with swapped args for compatibility if needed
};

} // namespace godot
