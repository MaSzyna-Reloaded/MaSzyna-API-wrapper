#include "E3DModelInstance.hpp"
#include "E3DModelInstanceManager.hpp"
#include "E3DNodesInstancer.hpp"
#include <godot_cpp/classes/engine.hpp>

namespace godot {
    const char *E3DModelInstance::E3D_LOADED_SIGNAL = "e3d_loaded";

    E3DModelInstance::E3DModelInstance()
        : data_path(String(), [this] { _reload(); })
        , model_filename(String(), [this] { _reload(); })
        , skins(Array(), [this] { _reload(); })
        , exclude_node_names(Array(), [this] { _reload(); })
        , instancer(INSTANCER_NODES, [this] { _reload(); })
        , submodels_aabb(AABB(), [this] { _reload(); })
        , editable_in_editor(false, [this] { _reload(); })
        {
        _mutex.instantiate();
    }

    E3DModelInstance::~E3DModelInstance() {
        _mutex.unref();
    }

    E3DModelInstanceManager *E3DModelInstance::_get_manager() const {
        return cast_to<E3DModelInstanceManager>(Engine::get_singleton()->get_singleton("E3DModelInstanceManager"));
    }

    void E3DModelInstance::_bind_methods() {
        ADD_SIGNAL(MethodInfo(E3D_LOADED_SIGNAL));
        BIND_PROPERTY(Variant::VECTOR3, "default_aabb_size", "default_aabb_size", &E3DModelInstance::set_default_aabb_size, &E3DModelInstance::get_default_aabb_size, "default_aabb_size");
        BIND_PROPERTY(Variant::STRING, "data_path", "data_path", &E3DModelInstance::set_data_path, &E3DModelInstance::get_data_path, "data_path");
        BIND_PROPERTY(Variant::STRING, "model_filename", "model_filename", &E3DModelInstance::set_model_filename, &E3DModelInstance::get_model_filename, "model_filename");
        BIND_PROPERTY_ARRAY("skins", "skins", &E3DModelInstance::set_skins, &E3DModelInstance::get_skins, "skins");
        BIND_PROPERTY_ARRAY("exclude_node_names", "exclude_node_names", &E3DModelInstance::set_exclude_node_names, &E3DModelInstance::get_exclude_node_names, "exclude_node_names");
        BIND_PROPERTY_W_HINT(Variant::INT, "instancer", "instancer", &E3DModelInstance::set_instancer, &E3DModelInstance::get_instancer, "instancer", PROPERTY_HINT_ENUM, "Optimized,Nodes,Editable nodes");
        BIND_PROPERTY(Variant::AABB, "submodels_aabb", "submodels_aabb", &E3DModelInstance::set_submodels_aabb, &E3DModelInstance::get_submodels_aabb, "submodels_aabb");
        BIND_PROPERTY(Variant::BOOL, "editable_in_editor", "editable_in_editor", &E3DModelInstance::set_editable_in_editor, &E3DModelInstance::get_editable_in_editor, "editable_in_editor");

        BIND_ENUM_CONSTANT(INSTANCER_NODES);
        BIND_ENUM_CONSTANT(INSTANCER_EDITABLE_NODES);
        BIND_ENUM_CONSTANT(INSTANCER_OPTIMIZED);

        ClassDB::bind_method(D_METHOD("_instantiate_children", "model"), &E3DModelInstance::_instantiate_children);
        ClassDB::bind_method(D_METHOD("_deferred_reload"), &E3DModelInstance::_deferred_reload);
        ClassDB::bind_method(D_METHOD("_flush_pending_model"), &E3DModelInstance::_flush_pending_model);
    }

    void E3DModelInstance::_notification(const int p_what) {
        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                UtilityFunctions::print_verbose("[E3DModelInstance] Entering tree");
                _reload();
                if (E3DModelInstanceManager *manager = _get_manager()) {
                    manager->register_instance(this);
                }
            } break;

            case NOTIFICATION_EXIT_TREE: {
                UtilityFunctions::print_verbose("[E3DModelInstance] Exiting tree");
                if (E3DModelInstanceManager *manager = _get_manager()) {
                    manager->unregister_instance(this);
                }
            } break;
            default: ;
        }
    }

    void E3DModelInstance::_reload() {
        if (_is_dirty) {
            return;
        }
        _is_dirty = true;
        call_deferred("_deferred_reload");
    }

    void E3DModelInstance::_deferred_reload() {
        _is_dirty = false;
        if (const E3DModelInstanceManager *manager = _get_manager()) {
            manager->reload_instance(this);
        }
    }

    void E3DModelInstance::_instantiate_children(const Ref<E3DModel> &p_model) {
        _mutex->lock();
        const bool is_scheduled = _pending_model_scheduled;
        _pending_model = p_model;
        _pending_model_scheduled = true;
        _mutex->unlock();

        if (is_scheduled) {
            return;
        }

        call_deferred("_flush_pending_model");
    }

    void E3DModelInstance::_flush_pending_model() {
        _mutex->lock();
        const Ref<E3DModel> model = _pending_model;
        _pending_model_scheduled = false;
        _mutex->unlock();

        E3DNodesInstancer::instantiate(model, this, get_editable_in_editor());
        emit_signal(E3D_LOADED_SIGNAL);
    }
}//namespace godot
