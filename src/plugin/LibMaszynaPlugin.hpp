// lib_maszyna_plugin.hpp

#ifndef LIB_MASZYNA_PLUGIN_HPP
#define LIB_MASZYNA_PLUGIN_HPP

#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

class LibMaszynaPlugin : public EditorPlugin {
        GDCLASS(LibMaszynaPlugin, EditorPlugin);

    public:
        // Constructor
        LibMaszynaPlugin();

        // Destructor
        ~LibMaszynaPlugin();

        virtual String get_name() const override {
            return "LibMaszynaPlugin";
        }
        virtual void edit(Object *p_node) override;
        virtual bool handles(Object *p_node) const override;

    protected:
        // Bind methods
        static void _bind_methods();
};

#endif // LIB_MASZYNA_PLUGIN_HPP
