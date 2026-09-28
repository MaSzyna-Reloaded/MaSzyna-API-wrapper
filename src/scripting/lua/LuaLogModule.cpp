#include "LuaModules.hpp"
#include "logging/GameLog.hpp"

namespace godot {
    namespace {
        int log_at(lua_State *p_state, const GameLog::LogLevel p_level) {
            const String line = String::utf8(luaL_checkstring(p_state, 1));
            LuaModules::server<GameLog>(p_state)->log(p_level, line);
            return 0;
        }

        int log_debug(lua_State *p_state) {
            return log_at(p_state, GameLog::DEBUG);
        }

        int log_info(lua_State *p_state) {
            return log_at(p_state, GameLog::INFO);
        }

        int log_warning(lua_State *p_state) {
            return log_at(p_state, GameLog::WARNING);
        }

        int log_error(lua_State *p_state) {
            return log_at(p_state, GameLog::ERROR);
        }
    } // namespace

    const luaL_Reg LuaModules::LOG[] = {
            {"debug", log_debug}, {"info", log_info}, {"warning", log_warning}, {"error", log_error}, {nullptr, nullptr},
    };
} // namespace godot
