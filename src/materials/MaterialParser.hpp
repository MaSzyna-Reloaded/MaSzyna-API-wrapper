#pragma once

#include "MaszynaMaterial.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

    class MaterialParser : public RefCounted {
            GDCLASS(MaterialParser, RefCounted)

        private:
            String _find_material_path(const String &p_model_path, const String &p_material_file) const;

        protected:
            static void _bind_methods();

        public:
            Ref<MaszynaMaterial> parse(const String &p_model_path, const String &p_material_file) const;
    };

} // namespace godot
