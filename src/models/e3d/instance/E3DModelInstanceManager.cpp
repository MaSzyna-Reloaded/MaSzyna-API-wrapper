#include "E3DModelInstance.hpp"
#include "E3DModelInstanceManager.hpp"
#include "E3DNodesInstancer.hpp"
#include "models/e3d/E3DModelManager.hpp"

#include <algorithm>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>

namespace godot {
    const char *E3DModelInstanceManager::instances_reloaded_signal = "instances_reloaded";
    void E3DModelInstanceManager::_bind_methods() {
        ADD_SIGNAL(MethodInfo(
                instances_reloaded_signal,
                PropertyInfo(Variant::OBJECT, "instance", PROPERTY_HINT_RESOURCE_TYPE, "E3DModelInstance")));
        ClassDB::bind_method(D_METHOD("reload_all"), &E3DModelInstanceManager::reload_all);
        ClassDB::bind_method(D_METHOD("register_instance", "instance"), &E3DModelInstanceManager::register_instance);
        ClassDB::bind_method(
                D_METHOD("unregister_instance", "instance"), &E3DModelInstanceManager::unregister_instance);
        ClassDB::bind_method(D_METHOD("reload_instance", "instance"), &E3DModelInstanceManager::reload_instance);
    }

    E3DModelInstanceManager::E3DModelInstanceManager() : model_manager(memnew(E3DModelManager)) {}

    E3DModelInstanceManager::~E3DModelInstanceManager() {
        if (model_manager != nullptr) {
            memdelete(model_manager);
            model_manager = nullptr;
        }
        instances.clear();
    }

    void E3DModelInstanceManager::reload_all() const {
        for (E3DModelInstance *instance: instances) {
            if (instance != nullptr) {
                reload_instance(instance);
            }
        }
    }

    void E3DModelInstanceManager::register_instance(E3DModelInstance *p_instance) {
        if (p_instance == nullptr) {
            return;
        }

        if (std::find(instances.begin(), instances.end(), p_instance) == instances.end()) {
            instances.push_back(p_instance);
        }
    }

    void E3DModelInstanceManager::unregister_instance(E3DModelInstance *p_instance) {
        if (p_instance == nullptr) {
            return;
        }

        if (const auto it = std::remove(instances.begin(), instances.end(), p_instance); it != instances.end()) {
            instances.erase(it, instances.end());
        }
    }

    void E3DModelInstanceManager::reload_instance(E3DModelInstance *p_instance) const {
        if (p_instance == nullptr) {
            return;
        }

        const SceneTree *tree = nullptr;
        if (p_instance->is_inside_tree()) {
            tree = p_instance->get_tree();
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
                model_manager->load_model(p_instance->get_data_path(), p_instance->get_model_filename());

        switch (p_instance->get_instancer()) {
            case E3DModelInstance::INSTANCER_OPTIMIZED: {
                UtilityFunctions::print_verbose(
                        "[E3DModelInstanceManager] Instantiating optimized E3D for ", p_instance->get_name());
                p_instance->call_deferred("_instantiate_children", model);
            } break;
            case E3DModelInstance::INSTANCER_NODES:
            case E3DModelInstance::INSTANCER_EDITABLE_NODES: {
                UtilityFunctions::print_verbose(
                        "[E3DModelInstanceManager] Instantiating nodes for ", p_instance->get_name());
                p_instance->call_deferred("_instantiate_children", model);
            } break;
            default:
                UtilityFunctions::push_warning(
                        "[E3DModelInstanceManager] Unsupported instancer type for ", p_instance->get_name());
                break;
        }
    }
} // namespace godot
