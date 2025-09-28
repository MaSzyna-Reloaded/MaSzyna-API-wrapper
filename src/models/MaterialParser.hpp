#pragma once
#include <godot_cpp/classes/node.hpp>
#include "resources/material/MaszynaMaterial.hpp"
#include "models/MaterialManager.hpp"

namespace godot {
    class MaszynaParser;
    class MaterialParser: public Node {
        GDCLASS(MaterialParser, Node)
        private:
            MaszynaParser *parser;
            MaterialManager *manager;
            const PackedStringArray STOP_KEY = {" ", ";", String::chr(9), String::chr(10), String::chr(13)};
            const PackedStringArray STOP_VALUE = {" ", ";", String::chr(9), String::chr(10), String::chr(13)};
        protected:
            static void _bind_methods();
        public:
        Ref<MaszynaMaterial> parse(const String &model_path, const String &material_path);
    };
} //namespace godot
