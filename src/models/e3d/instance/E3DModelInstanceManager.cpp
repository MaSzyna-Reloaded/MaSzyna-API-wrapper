#include "E3DModelInstanceManager.hpp"

#include "E3DModelInstance.hpp"
#include "E3DNodesInstancer.hpp"
#include "models/e3d/E3DModelManager.hpp"

#include <algorithm>

#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    const char *E3DModelInstanceManager::instances_reloaded_signal = "instances_reloaded";

    void E3DModelInstanceManager::_bind_methods() {
        ADD_SIGNAL(MethodInfo(
                instances_reloaded_signal,
                PropertyInfo(Variant::OBJECT, "instance", PROPERTY_HINT_NODE_TYPE, "E3DModelInstance")));

        ClassDB::bind_method(D_METHOD("reload_all"), &E3DModelInstanceManager::reload_all);

        ClassDB::bind_method(D_METHOD("register_instance", "instance"), &E3DModelInstanceManager::register_instance);

        ClassDB::bind_method(
                D_METHOD("unregister_instance", "instance"), &E3DModelInstanceManager::unregister_instance);

        ClassDB::bind_method(D_METHOD("reload_instance", "instance"), &E3DModelInstanceManager::reload_instance);
        ClassDB::bind_method(
                D_METHOD("teardown_all_for_extension_reload"),
                &E3DModelInstanceManager::teardown_all_for_extension_reload);
    }

    E3DModelInstanceManager::E3DModelInstanceManager() = default;

    E3DModelInstance *E3DModelInstanceManager::_get_instance(const ObjectID &p_instance_id) const {
        return Object::cast_to<E3DModelInstance>(ObjectDB::get_instance(p_instance_id));
    }

    void E3DModelInstanceManager::_compact_instances() {
        instances.erase(
                std::remove_if(
                        instances.begin(), instances.end(),
                        [this](const ObjectID &p_instance_id) { return _get_instance(p_instance_id) == nullptr; }),
                instances.end());
    }

    void E3DModelInstanceManager::reload_all() {
        if (extension_reload_in_progress) {
            return;
        }

        _compact_instances();

        const std::vector<ObjectID> instance_ids = instances;
        for (const ObjectID &instance_id: instance_ids) {
            E3DModelInstance *instance = _get_instance(instance_id);
            if (instance == nullptr) {
                continue;
            }

            if (instance->is_inside_tree()) {
                reload_instance(instance);
            }
        }
    }

    void E3DModelInstanceManager::register_instance(E3DModelInstance *p_instance) {
        if (p_instance == nullptr) {
            return;
        }

        _compact_instances();

        const ObjectID instance_id(p_instance->get_instance_id());
        if (std::find(instances.begin(), instances.end(), instance_id) != instances.end()) {
            return;
        }

        instances.push_back(instance_id);
    }

    void E3DModelInstanceManager::unregister_instance(E3DModelInstance *p_instance) {
        if (p_instance == nullptr) {
            _compact_instances();
            return;
        }

        const ObjectID instance_id(p_instance->get_instance_id());
        instances.erase(std::remove(instances.begin(), instances.end(), instance_id), instances.end());
    }

    void E3DModelInstanceManager::reload_instance(E3DModelInstance *p_instance) {
        E3DModelManager *model_manager = E3DModelManager::get_instance();

        if (extension_reload_in_progress || model_manager == nullptr || p_instance == nullptr ||
            !p_instance->is_inside_tree()) {
            return;
        }

        SceneTree *tree = p_instance->get_tree();

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

                p_instance->_instantiate_children(model);
            } break;

            case E3DModelInstance::INSTANCER_NODES:
            case E3DModelInstance::INSTANCER_EDITABLE_NODES: {
                UtilityFunctions::print_verbose(
                        "[E3DModelInstanceManager] Instantiating nodes for ", p_instance->get_name());

                p_instance->_instantiate_children(model);
            } break;

            default:
                UtilityFunctions::push_warning(
                        "[E3DModelInstanceManager] Unsupported instancer type for ", p_instance->get_name());
                return;
        }

        if (p_instance != nullptr && p_instance->is_inside_tree()) {
            emit_signal(instances_reloaded_signal, p_instance);
        }
    }

    void E3DModelInstanceManager::teardown_all_for_extension_reload() {
        extension_reload_in_progress = true;
        _compact_instances();

        const std::vector<ObjectID> instance_ids = instances;
        for (const ObjectID &instance_id: instance_ids) {
            E3DModelInstance *instance = _get_instance(instance_id);
            if (instance == nullptr) {
                continue;
            }

            instance->cleanup_for_extension_reload();
        }
    }

    void E3DModelInstanceManager::cleanup() {
        instances.clear();
        extension_reload_in_progress = false;
    }

} // namespace godot
