#include "maszyna_parser.hpp"

namespace godot {
    void MaszynaParser::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("initialize", "buffer", "default_stop"), &MaszynaParser::initialize, DEFVAL(Array()));
        ClassDB::bind_method(D_METHOD("get8"), &MaszynaParser::get8);
        ClassDB::bind_method(D_METHOD("get_line"), &MaszynaParser::get_line);
        ClassDB::bind_method(D_METHOD("eof_reached"), &MaszynaParser::eof_reached);
        ClassDB::bind_method(D_METHOD("register_handler", "token", "callback"), &MaszynaParser::register_handler);
        ClassDB::bind_method(D_METHOD("unregister_handler", "token"), &MaszynaParser::unregister_handler);
        ClassDB::bind_method(D_METHOD("as_bool", "token"), &MaszynaParser::as_bool);
        ClassDB::bind_method(D_METHOD("as_vector3", "tokens"), &MaszynaParser::as_vector3);
        ClassDB::bind_method(D_METHOD("get_tokens", "num", "stops"), &MaszynaParser::get_tokens, DEFVAL(Array()));
        ClassDB::bind_method(D_METHOD("next_token", "stops"), &MaszynaParser::next_token, DEFVAL(Array()));
        ClassDB::bind_method(D_METHOD("next_vector3", "stops"), &MaszynaParser::next_vector3, DEFVAL(Array()));
        ClassDB::bind_method(
                D_METHOD("get_tokens_until", "token", "stops"), &MaszynaParser::get_tokens_until, DEFVAL(Array()));
        ClassDB::bind_method(D_METHOD("parse"), &MaszynaParser::parse);
        ClassDB::bind_method(D_METHOD("get_parsed_metadata"), &MaszynaParser::get_parsed_metadata);
        ClassDB::bind_method(D_METHOD("push_metadata"), &MaszynaParser::push_metadata);
        ClassDB::bind_method(D_METHOD("pop_metadata"), &MaszynaParser::pop_metadata);
        ClassDB::bind_method(D_METHOD("clear_metadata"), &MaszynaParser::clear_metadata);
        ClassDB::bind_method(D_METHOD("set_parameters"), &MaszynaParser::set_parameters);
    }

    MaszynaParser::MaszynaParser() {
        default_stop_chars = Array::make(" ", "\t", "\n", "\r", ";");
        meta.push_back(Dictionary());
    }

    Array MaszynaParser::get_stops(const Array &p_stops) const {
        if (!p_stops.is_empty()) {
            return p_stops;
        }
        return default_stop_chars;
    }

    void MaszynaParser::set_parameters(const Dictionary &p_parameters) {
        parameters = p_parameters;
    }

    void MaszynaParser::initialize(const PackedByteArray &p_buffer, const Array &p_default_stop_chars) {
        this->buffer = p_buffer;
        length = static_cast<int>(p_buffer.size());
        cursor = 0;
        if (!p_default_stop_chars.is_empty()) {
            default_stop_chars = p_default_stop_chars;
        }
    }

    int MaszynaParser::get8() {
        if (cursor < length) {
            return buffer[cursor++];
        }
        return -1;
    }

    String MaszynaParser::get_line() {
        PackedByteArray subbuf;
        while (!eof_reached()) {
            const int c = get8();
            if (c == -1 || c == 10 || c == 13) {
                break;
            }
            subbuf.append(static_cast<uint8_t>(c));
        }
        return subbuf.get_string_from_utf8();
    }

    bool MaszynaParser::eof_reached() const {
        return cursor >= length;
    }

    void MaszynaParser::register_handler(const String &p_token, const Callable &p_callback) {
        handlers[p_token] = p_callback;
    }

    void MaszynaParser::unregister_handler(const String &p_token) {
        handlers.erase(p_token);
    }

    bool MaszynaParser::as_bool(const String &p_token) {
        const String lower = p_token.to_lower();
        return lower == "yes" || lower == "on" || lower == "1" || lower == "true" || lower == "vis";
    }

    Vector3 MaszynaParser::as_vector3(const Array &p_tokens) {
        if (p_tokens.size() < 3) {
            return Vector3();
        }
        return Vector3(p_tokens[0], p_tokens[1], p_tokens[2]);
    }

    Array MaszynaParser::get_tokens(const int p_num, const Array &p_stops) {
        const Array stop_chars = get_stops(p_stops);
        Array tokens;
        bool maybe_comment = false;
        bool maybe_endcomment = false;

        while (tokens.size() < p_num && !eof_reached()) {
            String token = "";

            while (!eof_reached()) {
                int c_int = get8();
                if (c_int == -1) {
                    break;
                }
                char c = static_cast<char>(c_int);
                bool skip = false;

                if (c == '/') {
                    if (maybe_comment) {
                        maybe_comment = false;
                        // Line comment detected
                        while (!eof_reached()) {
                            int next_c = get8();
                            if (next_c == -1 || next_c == '\n' || next_c == '\r') {
                                break;
                            }
                        }
                        skip = true;
                    } else {
                        maybe_comment = true;
                        continue;
                    }
                } else if (c == '*') {
                    if (maybe_comment) {
                        maybe_comment = false;
                        maybe_endcomment = false;
                        // Block comment detected
                        while (!eof_reached()) {
                            int next_c = get8();
                            if (next_c == -1) {
                                break;
                            }
                            char bc = static_cast<char>(next_c);
                            if (bc == '*') {
                                maybe_endcomment = true;
                            } else if (bc == '/' && maybe_endcomment) {
                                break;
                            } else {
                                maybe_endcomment = false;
                            }
                        }
                        break;
                    }
                }

                if (!skip && maybe_comment) {
                    maybe_comment = false;
                    token += '/';
                }

                if (skip || stop_chars.has(String::chr(c))) {
                    break;
                }

                token += c;
            }

            token = token.strip_edges();

            if (!token.is_empty()) {
                Array keys = parameters.keys();
                for (const auto &key: keys) {
                    String param = key;
                    String value = parameters[param];
                    token = token.replace("(" + param + ")", value);
                }
                tokens.append(token);
            }
        }

        return tokens;
    }

    String MaszynaParser::next_token(const Array &p_stops) {
        Array tokens = get_tokens(1, p_stops);
        return tokens.size() > 0 ? tokens[0] : "";
    }

    Vector3 MaszynaParser::next_vector3(const Array &p_stops) {
        const Array tokens = get_tokens(3, p_stops);
        return as_vector3(tokens);
    }

    Array MaszynaParser::get_tokens_until(const String &p_token, const Array &p_stops) {
        Array tokens;
        while (!eof_reached()) {
            if (String upcoming_token = next_token(p_stops); !upcoming_token.is_empty()) {
                tokens.append(upcoming_token);
                if (upcoming_token == p_token) {
                    break;
                }
            }
        }

        return tokens;
    }


    Array MaszynaParser::parse() {
        Array result;
        while (!eof_reached()) {
            if (String token = next_token(); handlers.has(token)) {
                if (Callable callback = handlers[token]; callback.is_valid()) {
                    Variant parsed_v = callback.call(this);
                    if (parsed_v.get_type() == Variant::ARRAY) {
                        Array parsed = parsed_v;
                        if (parsed.size() > 0) {
                            result.append_array(parsed);
                        }
                    }
                }
            }
        }

        return result;
    }

    Dictionary MaszynaParser::get_parsed_metadata() {
        if (meta.is_empty()) {
            return Dictionary();
        }
        return meta[0];
    }

    void MaszynaParser::push_metadata() {
        meta.push_front(Dictionary());
    }

    Dictionary MaszynaParser::pop_metadata() {
        if (meta.is_empty()) {
            return Dictionary();
        }
        Dictionary top = meta[0];
        meta.pop_front();
        return top;
    }

    void MaszynaParser::clear_metadata() {
        meta.clear();
        meta.push_back(Dictionary());
    }
} // namespace godot
