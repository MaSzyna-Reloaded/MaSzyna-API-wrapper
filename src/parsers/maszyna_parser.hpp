#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <vector>

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
            void initialize(const PackedByteArray &p_buffer);
            // void _create_instance(const PackedByteArray &buffer);
            int get8();
            String get_line();
            bool eof_reached() const;
            void register_handler(const String &p_token, const Callable &p_callback);
            bool as_bool(const String &p_token);
            Vector3 as_vector3(const Array &p_tokens);
            Array get_tokens(int p_num, const Array &p_stop);
            String next_token(const Array &p_stop);
            Vector3 next_vector3(const Array &p_stop);
            Array get_tokens_until(const String &p_token, const Array &p_stop);
            Array parse();
            Dictionary get_parsed_metadata();
            void push_metadata();
            Dictionary pop_metadata();
            void clear_metadata();
    };
} // namespace godot
