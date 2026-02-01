#include "E3DModelInstance.hpp"
#include "E3DModelInstanceManager.hpp"
#include "E3DNodesInstancer.hpp"
#include "core/ActionQueue.hpp"
#include "helpers/lambda.hpp"

#include "models/e3d/E3DModelManager.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>

#include <algorithm>

namespace godot {
    const char *E3DModelInstanceManager::INSTANCES_RELOADED_SIGNAL = "instances_reloaded";
    void E3DModelInstanceManager::_bind_methods() {
        ADD_SIGNAL(MethodInfo(
                INSTANCES_RELOADED_SIGNAL,
                PropertyInfo(Variant::OBJECT, "instance", PROPERTY_HINT_RESOURCE_TYPE, "E3DModelInstance")));
        ClassDB::bind_method(D_METHOD("reload_all"), &E3DModelInstanceManager::reload_all);
        ClassDB::bind_method(D_METHOD("register_instance", "instance"), &E3DModelInstanceManager::register_instance);
        ClassDB::bind_method(
                D_METHOD("unregister_instance", "instance"), &E3DModelInstanceManager::unregister_instance);
        ClassDB::bind_method(D_METHOD("reload_instance", "instance"), &E3DModelInstanceManager::reload_instance);
    }

    E3DModelInstanceManager::E3DModelInstanceManager() : model_manager(memnew(E3DModelManager)) {}

    E3DModelInstanceManager::~E3DModelInstanceManager() {
        memdelete(model_manager);
    }

    void E3DModelInstanceManager::reload_all() const {
        for (E3DModelInstance *instance: _instances) {
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
        if (const auto it =
                    std::remove(_instances.begin(), _instances.end(), instance);
            it != _instances.end()) {
            _instances.erase(it, _instances.end());
        }
    }

    void E3DModelInstanceManager::reload_instance(E3DModelInstance *instance) const {
        // Ensure we can access a SceneTree even when this object isn't inside the scene (e.g., running in editor).
        if (instance == nullptr) {
            return;
        }

        const SceneTree *tree = nullptr;
        if (instance->is_inside_tree()) {
            tree = instance->get_tree();
        }

        const Engine *singleton = Engine::get_singleton();

        if (tree == nullptr) {
            Object *ml = (singleton != nullptr) ? singleton->get_main_loop() : nullptr;
            tree = cast_to<SceneTree>(ml);
        }
        if (tree == nullptr) {
            UtilityFunctions::push_error("E3DModelInstanceManager::reload_instance: no SceneTree available");
            return;
        }

        const Ref<E3DModel> model =
                    model_manager->load_model(instance->get_data_path(), instance->get_model_filename());
        if (model.is_valid()) {
            switch (instance->get_instancer()) {
                case E3DModelInstance::INSTANCER_NODES: {
                    auto _do_instance = [instance, model]() -> void {
                        if (instance->is_queued_for_deletion()) {
                            return;
                        }
                        E3DNodesInstancer::instantiate(model, instance, instance->get_editable_in_editor());
                        instance->emit_signal(E3DModelInstance::E3D_LOADED_SIGNAL);
                    };
                    // We must instantiate nodes on the main thread
                    UtilityFunctions::print_verbose("[E3DModelInstanceManager] ", "Instantiating nodes");
                    make_lambda_callable(_do_instance).call_deferred();
                    instance->call_deferred("_instantiate_children", model);
                } break;
                default:
                    break;
            }
        }

    }
} // namespace godot
