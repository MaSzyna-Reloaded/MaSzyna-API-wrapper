#include "E3DModelInstance.hpp"

#include "E3DInstancer.hpp"
#include "E3DModelManager.hpp"
#include "E3DModelTool.hpp"
#include "E3DNodesInstancer.hpp"
#include "E3DOptimizedInstancer.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    void E3DModelInstance::_bind_methods() {
        ClassDB::bind_method(D_METHOD("reload"), &E3DModelInstance::reload);
        ClassDB::bind_method(D_METHOD("is_e3d_loaded"), &E3DModelInstance::is_e3d_loaded);

        ClassDB::bind_method(D_METHOD("set_model", "model"), &E3DModelInstance::set_model);
        ClassDB::bind_method(D_METHOD("get_model"), &E3DModelInstance::get_model);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "model", PROPERTY_HINT_RESOURCE_TYPE, "E3DModel"), "set_model",
                "get_model");

        BIND_PROPERTY(
                Variant::STRING, "data_path", "data_path", &E3DModelInstance::set_data_path,
                &E3DModelInstance::get_data_path, "data_path");
        BIND_PROPERTY(
                Variant::STRING, "model_filename", "model_filename", &E3DModelInstance::set_model_filename,
                &E3DModelInstance::get_model_filename, "model_filename");
        BIND_PROPERTY_ARRAY("skins", "skins", &E3DModelInstance::set_skins, &E3DModelInstance::get_skins, "skins");
        BIND_PROPERTY_ARRAY(
                "exclude_node_names", "exclude_node_names", &E3DModelInstance::set_exclude_node_names,
                &E3DModelInstance::get_exclude_node_names, "exclude_node_names");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "instancer", "instancer", &E3DModelInstance::set_instancer,
                &E3DModelInstance::get_instancer, "instancer", PROPERTY_HINT_ENUM, "OPTIMIZED,NODES,EDITABLE_NODES");
        BIND_PROPERTY(
                Variant::BOOL, "editable_in_editor", "editable_in_editor", &E3DModelInstance::set_editable_in_editor,
                &E3DModelInstance::is_editable_in_editor, "editable_in_editor");

        ADD_SIGNAL(MethodInfo("e3d_loading"));
        ADD_SIGNAL(MethodInfo("e3d_loaded"));

        BIND_ENUM_CONSTANT(OPTIMIZED);
        BIND_ENUM_CONSTANT(NODES);
        BIND_ENUM_CONSTANT(EDITABLE_NODES);
    }

    E3DModelInstance::E3DModelInstance() {
        set_process(true);
    }

    void E3DModelInstance::_notification(int p_what) {
        switch (p_what) {
            case NOTIFICATION_PROCESS:
                if (Engine::get_singleton()->is_editor_hint() && dirty) {
                    _process_dirty();
                    dirty = false;
                }
                break;
            case NOTIFICATION_READY:
                set_notify_transform(true);
                reload();
                break;
            case NOTIFICATION_ENTER_TREE:
                if (loaded_model.is_valid() && current_instancer != nullptr) {
                    current_instancer->instantiate(this, loaded_model, current_editable);
                }
                break;
            case NOTIFICATION_EXIT_TREE:
                if (current_instancer != nullptr) {
                    current_instancer->clear(this);
                }
                break;
            case NOTIFICATION_TRANSFORM_CHANGED:
            case NOTIFICATION_VISIBILITY_CHANGED:
                if (current_instancer != nullptr) {
                    current_instancer->sync(this);
                }
                break;
            default:
                break;
        }
    }

    AABB E3DModelInstance::_get_aabb() const {
        return submodels_aabb;
    }

    Ref<E3DModel> E3DModelInstance::get_model() const {
        return model;
    }

    void E3DModelInstance::set_model(const Ref<E3DModel> &p_model) {
        if (model == p_model) {
            return;
        }
        model = p_model;
        dirty = true;
    }

    String E3DModelInstance::get_data_path() const {
        return data_path;
    }

    void E3DModelInstance::set_data_path(const String &p_data_path) {
        if (data_path == p_data_path) {
            return;
        }
        data_path = p_data_path;
        dirty = true;
    }

    String E3DModelInstance::get_model_filename() const {
        return model_filename;
    }

    void E3DModelInstance::set_model_filename(const String &p_model_filename) {
        if (model_filename == p_model_filename) {
            return;
        }
        model_filename = p_model_filename;
        dirty = true;
    }

    Array E3DModelInstance::get_skins() const {
        return skins;
    }

    void E3DModelInstance::set_skins(const Array &p_skins) {
        if (skins == p_skins) {
            return;
        }
        skins = p_skins;
        dirty = true;
    }

    Array E3DModelInstance::get_exclude_node_names() const {
        return exclude_node_names;
    }

    void E3DModelInstance::set_exclude_node_names(const Array &p_exclude_node_names) {
        if (exclude_node_names == p_exclude_node_names) {
            return;
        }
        exclude_node_names = p_exclude_node_names;
        dirty = true;
    }

    E3DModelInstance::Instancer E3DModelInstance::get_instancer() const {
        return instancer;
    }

    void E3DModelInstance::set_instancer(const Instancer p_instancer) {
        if (instancer == p_instancer) {
            return;
        }
        instancer = p_instancer;
        dirty = true;
    }

    bool E3DModelInstance::is_editable_in_editor() const {
        return editable_in_editor;
    }

    void E3DModelInstance::set_editable_in_editor(const bool p_editable_in_editor) {
        if (editable_in_editor == p_editable_in_editor) {
            return;
        }
        editable_in_editor = p_editable_in_editor;
        dirty = true;
    }

    E3DInstancer *E3DModelInstance::_resolve_instancer() const {
        if (instancer == Instancer::OPTIMIZED) {
            return E3DOptimizedInstancer::get_instance();
        }
        return E3DNodesInstancer::get_instance();
    }

    void E3DModelInstance::_process_dirty() {
        reload();
    }

    void E3DModelInstance::reload() {
        if (!is_inside_tree() || (model.is_null() && model_filename.is_empty())) {
            return;
        }

        e3d_loaded = false;
        emit_signal("e3d_loading");

        if (current_instancer != nullptr) {
            current_instancer->clear(this);
        }

        current_instancer = _resolve_instancer();

        if (model.is_valid()) {
            loaded_model = model;
        } else {
            E3DModelManager *manager = E3DModelManager::get_instance();
            loaded_model = manager != nullptr ? manager->load_model(data_path, model_filename) : Ref<E3DModel>();
        }

        if (loaded_model.is_null()) {
            return;
        }

        E3DModelTool *tool = E3DModelTool::get_instance();
        submodels_aabb = tool != nullptr ? tool->get_aabb(loaded_model) : AABB();
        current_editable = editable_in_editor || instancer == Instancer::EDITABLE_NODES;

        if (current_instancer == nullptr) {
            UtilityFunctions::push_error("Selected instancer is not supported!");
            return;
        }

        current_instancer->clear(this);
        current_instancer->instantiate(this, loaded_model, current_editable);
        e3d_loaded = true;
        emit_signal("e3d_loaded");
    }

    bool E3DModelInstance::is_e3d_loaded() const {
        return e3d_loaded;
    }

} // namespace godot
