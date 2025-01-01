#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {
    class MaszynaParser : public Object {
            GDCLASS(MaszynaParser, Object);

        private:
            PackedByteArray buffer;
            Dictionary handlers;
            int cursor = 0;
            int length = 0;
            std::vector<Dictionary> meta;
            Dictionary parameters;

        protected:
            static void _bind_methods();

        public:
            MaszynaParser();
            void initialize(const PackedByteArray &buffer);
            // void _create_instance(const PackedByteArray &buffer);
            int get8();
            String get_line();
            bool eof_reached();
            void register_handler(const String &token, const Callable &callback);
            bool as_bool(const String &token);
            Vector3 as_vector3(const Array &tokens);
            Array get_tokens(int num, const Array &stop);
            String next_token(const Array &stop);
            Vector3 next_vector3(const Array &stop);
            Array get_tokens_until(const String &token, const Array &stop);
            Array parse();
            Dictionary get_parsed_metadata();
            void push_metadata();
            Dictionary pop_metadata();
            void clear_metadata();
    };
} // namespace godot
