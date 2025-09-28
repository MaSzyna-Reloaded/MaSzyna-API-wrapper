#include "core/ActionQueue.hpp"
#include "E3DModelInstance.hpp"
#include "E3DModelInstanceManager.hpp"
#include "E3DModelNodesInstancer.hpp"
#include "helpers/lambda.hpp"

#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include "models/e3d/E3DModelManager.hpp"

#include <algorithm>

namespace godot {
    const char *E3DModelInstanceManager::INSTANCES_RELOADED_SIGNAL = "instances_reloaded";
    const char *E3DModelInstanceManager::E3D_LOADED_SIGNAL = "e3d_loaded";
    void E3DModelInstanceManager::_bind_methods() {
        ADD_SIGNAL(MethodInfo(E3D_LOADED_SIGNAL));
        ADD_SIGNAL(MethodInfo(INSTANCES_RELOADED_SIGNAL, PropertyInfo(Variant::OBJECT, "instance", PROPERTY_HINT_RESOURCE_TYPE, "E3DModelInstance")));
    }

    E3DModelInstanceManager::E3DModelInstanceManager() {
        model_manager = memnew(E3DModelManager);
    }

    E3DModelInstanceManager::~E3DModelInstanceManager() {
        memdelete(model_manager);
    }

    void E3DModelInstanceManager::reload_all() {
        for (E3DModelInstance *instance : _instances) {
            if (instance != nullptr) {
                reload_instance(instance);
            }
        }
    }

    void E3DModelInstanceManager::register_instance(E3DModelInstance *instance) {
        if (instance == nullptr) {
            return;
        }
        if (std::find(_instances.begin(), _instances.end(), instance) == _instances.end()) {
            _instances.push_back(instance);
        }
    }

    void E3DModelInstanceManager::unregister_instance(E3DModelInstance *instance) {
        if (instance == nullptr) {
            return;
        }
        if (const __gnu_cxx::__normal_iterator<E3DModelInstance **, std::vector<E3DModelInstance *>> it = std::remove(
                _instances.begin(), _instances.end(), instance); it != _instances.end()) {
            _instances.erase(it, _instances.end());
        }
    }

    void E3DModelInstanceManager::reload_instance(E3DModelInstance *instance) {
        // Ensure we can access a SceneTree even when this object isn't inside the scene (e.g., running in editor).
        const SceneTree *tree = get_tree();
        if (tree == nullptr) {
            Object *ml = Engine::get_singleton() ? Engine::get_singleton()->get_main_loop() : nullptr;
            tree = cast_to<SceneTree>(ml);
        }
        if (tree == nullptr) {
            UtilityFunctions::push_error("E3DModelInstanceManager::reload_instance: no SceneTree available");
            return;
        }

        // ActionQueue is an Engine singleton (RefCounted), not a Node in the scene.
        ActionQueue *action_queue = cast_to<ActionQueue>(Engine::get_singleton()->get_singleton("ActionQueue"));
        if (!action_queue) {
            UtilityFunctions::push_error("E3DModelInstanceManager::reload_instance: ActionQueue singleton not found");
            return;
        }
        if (!instance) return;
        auto _do_load = [instance, this, action_queue] () -> void {
            const Ref<E3DModel> model = model_manager->load_model(instance->get_data_path(), instance->get_model_filename());
            if (model.is_valid()) {
                switch (instance->get_instancer()) {
                    case E3DModelInstance::INSTANCER_NODES: {
                        auto _do_instance = [this, instance, model] () -> void {
                            if (instance->is_queued_for_deletion())
                                return;
                            const E3DModel *_model = model.ptr();
                            E3DModelNodesInstancer::instantiate(*_model, *instance, instance->get_editable_in_editor());
                            emit_signal(E3D_LOADED_SIGNAL);
                        };
                        // We must instantiate nodes on the main thread
                        action_queue->call_deferred("add_item", make_lambda_callable(_do_instance), "Creating model");
                    } break;
                    default: break;
                }
            };
        };

        WorkerThreadPool::get_singleton()->add_task(make_lambda_callable(_do_load), true, "Reloading E3D model");
    }
} //namespace godot
