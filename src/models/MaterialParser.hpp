#pragma once

#include "resources/material/MaszynaMaterial.hpp"
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {
    class MaterialParser: public RefCounted {
        GDCLASS(MaterialParser, RefCounted)
        private:
            const PackedStringArray stop_key = {" ", ";", String::chr(9), String::chr(10), String::chr(13)};
            const PackedStringArray stop_value = {" ", ";", String::chr(9), String::chr(10), String::chr(13)};
        protected:
            static void _bind_methods();
        public:
            Ref<MaszynaMaterial> parse(const String &p_material_path, const String &p_material_name) const;
    };
} //namespace godot