#include "E3DModelInstance.hpp"
#include "E3DModelInstanceManager.hpp"
namespace godot {

    E3DModelInstance::E3DModelInstance()
        : _manager(memnew(E3DModelInstanceManager))
        , data_path(String(), [this] { _reload(); })
        , model_filename(String(), [this] { _reload(); })
        , skins(Array(), [this] { _reload(); })
        , exclude_node_names(Array(), [this] { _reload(); })
        , instancer(INSTANCER_NODES, [this] { _reload(); })
        , submodels_aabb(AABB(), [this] { _reload(); })
        , editable_in_editor(false, [this] { _reload(); }) {
    }

    E3DModelInstance::~E3DModelInstance() {
        memdelete(_manager);
    }

    void E3DModelInstance::_bind_methods() {
        BIND_PROPERTY(Variant::VECTOR3, "default_aabb_size", "default_aabb_size", &E3DModelInstance::set_default_aabb_size, &E3DModelInstance::get_default_aabb_size, "default_aabb_size");
        BIND_PROPERTY(Variant::STRING, "data_path", "data_path", &E3DModelInstance::set_data_path, &E3DModelInstance::get_data_path, "data_path");
        BIND_PROPERTY(Variant::STRING, "model_filename", "model_filename", &E3DModelInstance::set_model_filename, &E3DModelInstance::get_model_filename, "model_filename");
        BIND_PROPERTY_ARRAY("skins", "skins", &E3DModelInstance::set_skins, &E3DModelInstance::get_skins, "skins");
        BIND_PROPERTY_ARRAY("exclude_node_names", "exclude_node_names", &E3DModelInstance::set_exclude_node_names, &E3DModelInstance::get_exclude_node_names, "exclude_node_names");
        BIND_PROPERTY_W_HINT(Variant::INT, "instancer", "instancer", &E3DModelInstance::set_instancer, &E3DModelInstance::get_instancer, "instancer", PROPERTY_HINT_ENUM, "Optimized,Nodes,Editable nodes");
        BIND_PROPERTY(Variant::AABB, "submodels_aabb", "submodels_aabb", &E3DModelInstance::set_submodels_aabb, &E3DModelInstance::get_submodels_aabb, "submodels_aabb");
        BIND_PROPERTY(Variant::BOOL, "editable_in_editor", "editable_in_editor", &E3DModelInstance::set_editable_in_editor, &E3DModelInstance::get_editable_in_editor, "editable_in_editor");
}

    void E3DModelInstance::_notification(const int p_what) {
        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                _reload();
                _manager->register_instance(this);
            };

            case NOTIFICATION_EXIT_TREE: {
                _manager->unregister_instance(this);
            };
            default: ;
        }
    }

    void E3DModelInstance::_reload() {
        _manager->reload_instance(this);
    }
}//namespace godot

