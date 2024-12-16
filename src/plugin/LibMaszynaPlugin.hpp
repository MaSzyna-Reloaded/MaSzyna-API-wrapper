// lib_maszyna_plugin.hpp

#ifndef LIB_MASZYNA_PLUGIN_HPP
#define LIB_MASZYNA_PLUGIN_HPP

#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class LibMaszynaPlugin final : public EditorPlugin {
        // NOLINTNEXTLINE(modernize-use-auto)
        GDCLASS(LibMaszynaPlugin, EditorPlugin);

    public:
        // Constructor
        LibMaszynaPlugin();

        // Destructor
        ~LibMaszynaPlugin() override;

        static String get_name() {
            return "LibMaszynaPlugin";
        }
        void _edit(Object *p_node) override;
        bool _handles(Object *p_node) const override;

    protected:
        // Bind methods
        static void _bind_methods();
};

#endif // LIB_MASZYNA_PLUGIN_HPP
