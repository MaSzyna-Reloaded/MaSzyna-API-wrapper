#include "SceneryLoadingTaskQueue.hpp"
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/core/mutex_lock.hpp>

namespace godot {
    void SceneryLoadingTaskQueue::_bind_methods() {
        ClassDB::bind_method(D_METHOD("submit", "task"), &SceneryLoadingTaskQueue::submit);
        ClassDB::bind_method(D_METHOD("is_done", "task_id"), &SceneryLoadingTaskQueue::is_done);
        ClassDB::bind_method(D_METHOD("wait", "task_id"), &SceneryLoadingTaskQueue::wait);
        ClassDB::bind_method(D_METHOD("get_completed_count"), &SceneryLoadingTaskQueue::get_completed_count);
        ClassDB::bind_method(D_METHOD("get_worker_count"), &SceneryLoadingTaskQueue::get_worker_count);
    }

    SceneryLoadingTaskQueue::SceneryLoadingTaskQueue() {
        mutex.instantiate();
        semaphore.instantiate();
    }

    /// Started with the first task, not in the constructor (the object is fully set up then)
    void SceneryLoadingTaskQueue::_start_workers() {
        for (int i = 0; i < get_worker_count(); i++) {
            Ref<Thread> worker;
            worker.instantiate();
            worker->start(callable_mp(this, &SceneryLoadingTaskQueue::_worker_loop));
            workers.push_back(worker);
        }
    }

    /// Queued tasks are dropped, running ones are finished before the workers are joined.
    SceneryLoadingTaskQueue::~SceneryLoadingTaskQueue() {
        {
            MutexLock lock(**mutex);
            exiting = true;
            pending.clear();
        }
        semaphore->post(static_cast<int32_t>(workers.size()));
        for (const Ref<Thread> &worker: workers) {
            worker->wait_to_finish();
        }
    }

    int SceneryLoadingTaskQueue::submit(const Callable &p_task) {
        if (workers.is_empty()) {
            _start_workers();
        }
        int task_id = 0;
        {
            MutexLock lock(**mutex);
            task_id = next_id++;
            tasks[task_id].callable = p_task;
            pending.push_back(task_id);
        }
        semaphore->post();
        return task_id;
    }

    bool SceneryLoadingTaskQueue::is_done(const int p_task_id) const {
        MutexLock lock(**mutex);
        const Task *task = tasks.getptr(p_task_id);
        return task != nullptr && task->done;
    }

    /// Returns the task result and forgets the task; runs other queued tasks until it is done.
    Variant SceneryLoadingTaskQueue::wait(const int p_task_id) {
        while (true) {
            {
                MutexLock lock(**mutex);
                const HashMap<int, Task>::Iterator task = tasks.find(p_task_id);
                ERR_FAIL_COND_V_MSG(task == tasks.end(), Variant(), vformat("Unknown task id: %d", p_task_id));
                if (task->value.done) {
                    Variant result = task->value.result;
                    tasks.remove(task);
                    return result;
                }
            }
            if (!_run_next()) {
                // the task runs on another thread
                OS::get_singleton()->delay_usec(100);
            }
        }
    }

    int SceneryLoadingTaskQueue::get_completed_count() const {
        MutexLock lock(**mutex);
        return completed;
    }

    int SceneryLoadingTaskQueue::get_worker_count() const {
        return MAX(OS::get_singleton()->get_processor_count() - 2, 1);
    }

    /// Runs the oldest queued task, false when there is none.
    bool SceneryLoadingTaskQueue::_run_next() {
        Callable callable;
        int task_id = 0;
        {
            MutexLock lock(**mutex);
            if (pending.is_empty()) {
                return false;
            }
            task_id = pending.front()->get();
            pending.pop_front();
            Task &task = tasks[task_id];
            callable = task.callable;
            task.callable = Callable();
        }

        const Variant result = callable.call();
        // Drop the task's references (it may hold this queue) before it is reported as done, so the
        // last reference to the queue is never released on a worker thread (joining itself)
        callable = Callable();

        MutexLock lock(**mutex);
        Task &task = tasks[task_id];
        task.result = result;
        task.done = true;
        completed++;
        return true;
    }

    void SceneryLoadingTaskQueue::_worker_loop() {
        while (true) {
            semaphore->wait();
            {
                MutexLock lock(**mutex);
                if (exiting) {
                    return;
                }
            }
            // a waiting thread may have taken the task already
            _run_next();
        }
    }
} // namespace godot
