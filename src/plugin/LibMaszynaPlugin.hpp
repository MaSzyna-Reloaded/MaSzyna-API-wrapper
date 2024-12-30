// lib_maszyna_plugin.hpp

#ifndef LIB_MASZYNA_PLUGIN_HPP
#define LIB_MASZYNA_PLUGIN_HPP

#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class LibMaszynaPlugin: public EditorPlugin {
        GDCLASS(LibMaszynaPlugin, EditorPlugin);

    public:
        String get_name() const {
            return "LibMaszynaPlugin";
        }
        void _edit(Object *p_node) override;
        bool _handles(Object *p_node) const override;

    protected:
        // Bind methods
        static void _bind_methods();
};

#endif // LIB_MASZYNA_PLUGIN_HPP
