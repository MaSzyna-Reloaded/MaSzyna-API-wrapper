#pragma once
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/semaphore.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/list.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/callable.hpp>

namespace godot {
    /// FIFO of tasks (Callables) run by max(processor count - 2, 1) worker threads.
    /// A task may submit further tasks and wait() for them: the waiting thread runs the task it
    /// waits for, so nested waiting never deadlocks the workers and never nests deeper than the
    /// tasks themselves. Used to parse scenery includes.
    class SceneryLoadingTaskQueue : public RefCounted {
            GDCLASS(SceneryLoadingTaskQueue, RefCounted)

        private:
            struct Task {
                    Callable callable;
                    Variant result;
                    bool done = false;
            };

            Ref<Mutex> mutex;
            Ref<Semaphore> semaphore;
            Vector<Ref<Thread>> workers;
            HashMap<int, Task> tasks;
            List<int> pending;
            int next_id = 0;
            int completed = 0;
            bool exiting = false;

            void _start_workers();
            bool _run_next();
            bool _run_task(int p_task_id);
            Callable _take_callable(int p_task_id);
            void _run(int p_task_id, Callable &p_callable);
            void _worker_loop();

        protected:
            static void _bind_methods();

        public:
            SceneryLoadingTaskQueue();
            ~SceneryLoadingTaskQueue() override;

            int submit(const Callable &p_task);
            bool is_done(int p_task_id) const;
            Variant wait(int p_task_id);
            int get_completed_count() const;
            int get_worker_count() const;
    };
} // namespace godot
