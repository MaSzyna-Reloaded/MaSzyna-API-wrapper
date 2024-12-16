#include "maszyna_parser.hpp"

namespace godot {
    void MaszynaParser::_bind_methods() {
        ClassDB::bind_method(D_METHOD("initialize", "buffer"), &MaszynaParser::initialize);
        ClassDB::bind_method(D_METHOD("get8"), &MaszynaParser::get8);
        ClassDB::bind_method(D_METHOD("get_line"), &MaszynaParser::get_line);
        ClassDB::bind_method(D_METHOD("eof_reached"), &MaszynaParser::eof_reached);
        ClassDB::bind_method(D_METHOD("register_handler", "token", "callback"), &MaszynaParser::register_handler);
        ClassDB::bind_method(D_METHOD("as_bool", "token"), &MaszynaParser::as_bool);
        ClassDB::bind_method(D_METHOD("as_vector3", "tokens"), &MaszynaParser::as_vector3);
        ClassDB::bind_method(D_METHOD("get_tokens", "num", "stop"), &MaszynaParser::get_tokens);
        ClassDB::bind_method(D_METHOD("next_token", "stop"), &MaszynaParser::next_token);
        ClassDB::bind_method(D_METHOD("next_vector3", "stop"), &MaszynaParser::next_vector3);
        ClassDB::bind_method(D_METHOD("get_tokens_until", "token", "stop"), &MaszynaParser::get_tokens_until);
        ClassDB::bind_method(D_METHOD("parse"), &MaszynaParser::parse);
        ClassDB::bind_method(D_METHOD("get_parsed_metadata"), &MaszynaParser::get_parsed_metadata);
        ClassDB::bind_method(D_METHOD("push_metadata"), &MaszynaParser::push_metadata);
        ClassDB::bind_method(D_METHOD("pop_metadata"), &MaszynaParser::pop_metadata);
        ClassDB::bind_method(D_METHOD("clear_metadata"), &MaszynaParser::clear_metadata);
    }

    MaszynaParser::MaszynaParser() {
        meta.push_back(Dictionary());
    }

    void MaszynaParser::initialize(const PackedByteArray &buffer) {
        this->buffer = buffer;
        length = buffer.size();
        cursor = 0;
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
            int c = get8();
            if (c == 10 || c == 13) {
                break;
            }
            subbuf.append(c);
        }
        return String::utf8((const char *)subbuf.ptr(), subbuf.size());
    }

    bool MaszynaParser::eof_reached() {
        return cursor >= length;
    }

    void MaszynaParser::register_handler(const String &token, const Callable &callback) {
        handlers[token] = callback;
    }

    bool MaszynaParser::as_bool(const String &token) {
        String lower = token.to_lower();
        return lower == "yes" || lower == "on" || lower == "1" || lower == "true" || lower == "vis";
    }

    Vector3 MaszynaParser::as_vector3(const Array &tokens) {
        return Vector3(tokens[0], tokens[1], tokens[2]);
    }

    Array MaszynaParser::get_tokens(int num, const Array &stop) {
        Array tokens;
        String token;
        bool maybe_comment = false;
        bool maybe_endcomment = false;

        while (tokens.size() < num && !eof_reached()) {
            token = "";

            while (!eof_reached()) {
                char c = (char)get8();
                bool skip = false;

                if (c == '/') {
                    if (maybe_comment) {
                        // Line comment detected
                        while (!eof_reached() && c != '\n' && c != '\r') {
                            c = (char)get8();
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
                            c = (char)get8();
                            if (c == '*') {
                                maybe_endcomment = true;
                            } else if (c == '/' && maybe_endcomment) {
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

                if (skip || stop.has(String::chr(c))) {
                    break;
                }

                token += c;
            }

            token = token.strip_edges();

            if (!token.is_empty()) {
                Array keys = parameters.keys();
                for (int i = 0; i < keys.size(); ++i) {
                    String param = keys[i];
                    String value = parameters[param];
                    token = token.replace("(" + param + ")", value);
                }
                tokens.append(token);
            }
        }

        return tokens;
    }

    String MaszynaParser::next_token(const Array &stop = Array::make(" ", '\t', '\n', '\r', ';')) {
        Array tokens = get_tokens(1, stop);
        return tokens.size() > 0 ? tokens[0] : "";
    }

    Vector3 MaszynaParser::next_vector3(const Array &stop) {
        Array tokens = get_tokens(3, stop);
        return as_vector3(tokens);
    }

    Array
    MaszynaParser::get_tokens_until(const String &token, const Array &stop = Array::make(" ", '\t', '\n', '\r', ';')) {
        Array tokens;

        while (!eof_reached()) {
            String upcoming_token = next_token(stop);
            if (!upcoming_token.is_empty()) {
                tokens.append(upcoming_token);
                if (upcoming_token == token) {
                    break;
                }
            }
        }

        return tokens;
    }


    Array MaszynaParser::parse() {
        Array result;

        while (!eof_reached()) {
            String token = next_token();

            if (handlers.has(token)) {
                Callable callback = handlers[token];
                if (callback.is_valid()) {
                    Array parsed = callback.call();
                    if (parsed.size() > 0) {
                        result.append_array(parsed);
                    }
                }
            }
        }

        return result;
    }

    Dictionary MaszynaParser::get_parsed_metadata() {
        return meta.front();
    }

    void MaszynaParser::push_metadata() {
        meta.insert(meta.begin(), Dictionary());
    }

    Dictionary MaszynaParser::pop_metadata() {
        Dictionary top = meta.front();
        meta.erase(meta.begin());
        return top;
    }

    void MaszynaParser::clear_metadata() {
        meta.clear();
        meta.push_back(Dictionary());
    }
} // namespace godot
