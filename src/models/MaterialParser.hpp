#pragma once
#include "models/MaterialManager.hpp"
#include "resources/material/MaszynaMaterial.hpp"
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {
    class MaszynaParser;
    class MaterialParser: public RefCounted {
        GDCLASS(MaterialParser, RefCounted)
        private:
            const PackedStringArray STOP_KEY = {" ", ";", String::chr(9), String::chr(10), String::chr(13)};
            const PackedStringArray STOP_VALUE = {" ", ";", String::chr(9), String::chr(10), String::chr(13)};
        protected:
            static void _bind_methods();
        public:
            Ref<MaszynaMaterial> parse(MaterialManager *material_manager, const String &model_path, const String &material_path) const;
    };
} //namespace godot
