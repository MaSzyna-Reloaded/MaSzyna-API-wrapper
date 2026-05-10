#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <vector>

namespace godot {
    class MaszynaParser : public RefCounted {
            GDCLASS(MaszynaParser, RefCounted);

        private:
            PackedByteArray buffer;
            Dictionary handlers;
            int cursor = 0;
            int length = 0;
            TypedArray<Dictionary> meta;
            Array default_stop_chars;
            Dictionary parameters;
            Array get_stops(const Array &p_stops) const;

        protected:
            static void _bind_methods();

        public:
            MaszynaParser();
            void initialize(const PackedByteArray &p_buffer, const Array &p_default_stop_chars);
            int get8();
            String get_line();
            bool eof_reached() const;
            void register_handler(const String &p_token, const Callable &p_callback);
            void unregister_handler(const String &p_token);
            void set_parameters(const Dictionary &p_parameters);
            bool as_bool(const String &p_token);
            Vector3 as_vector3(const Array &p_tokens);
            Array get_tokens(int p_num, const Array &p_stops = Array());
            String next_token(const Array &p_stops = Array());
            Vector3 next_vector3(const Array &p_stops = Array());
            Array get_tokens_until(const String &p_token, const Array &p_stops = Array());
            Array parse();
            Dictionary get_parsed_metadata();
            void push_metadata();
            Dictionary pop_metadata();
            void clear_metadata();
    };
} // namespace godot
