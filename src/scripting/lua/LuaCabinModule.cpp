#include "LuaHandle.hpp"
#include "LuaModules.hpp"
#include "LuaScriptContext.hpp"
#include "LuaVariant.hpp"

namespace godot {
    /// The cab layer of this script's context; raises an error when none was attached
    static Ref<ScenarioScriptCabinDelegate> cabin(lua_State *p_state) {
        const Ref<ScenarioScriptCabinDelegate> delegate =
                LuaModules::server<ScenarioScriptServer>(p_state)->context_get_cabin_delegate(
                        LuaScriptContext::from_state(p_state)->get_rid());
        if (delegate.is_null()) {
            // luaL_error is the Lua C API's vararg error call
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            luaL_error(p_state, "no cabs are available to scripts here");
        }
        return delegate;
    }

    /// act(v, cab, control_id, action, value) - manipulates a control of the vehicle's cab
    /// as the driver's hand does; action is "increase", "decrease", "hold", "release",
    /// "toggle" or "set". Returns what the control answered.
    static int cabin_act(lua_State *p_state) {
        const RID vehicle = LuaHandle::check(p_state, 1, ScriptHandleKind::VEHICLE);
        const int cab = static_cast<int>(luaL_checkinteger(p_state, 2));
        const StringName control_id = String::utf8(luaL_checkstring(p_state, 3));
        const StringName action = String::utf8(luaL_checkstring(p_state, 4));
        const Variant value = LuaVariant::to_variant(p_state, 5);
        LuaVariant::push(p_state, cabin(p_state)->act(vehicle, cab, control_id, action, value));
        return 1;
    }

    /// control(v, cab, control_id) - where the control stands
    static int cabin_control(lua_State *p_state) {
        const RID vehicle = LuaHandle::check(p_state, 1, ScriptHandleKind::VEHICLE);
        const int cab = static_cast<int>(luaL_checkinteger(p_state, 2));
        const StringName control_id = String::utf8(luaL_checkstring(p_state, 3));
        LuaVariant::push(p_state, cabin(p_state)->get_control(vehicle, cab, control_id));
        return 1;
    }

    /// controls(v, cab) - the ids of the cab's controls
    static int cabin_controls(lua_State *p_state) {
        const RID vehicle = LuaHandle::check(p_state, 1, ScriptHandleKind::VEHICLE);
        const int cab = static_cast<int>(luaL_checkinteger(p_state, 2));
        LuaVariant::push(p_state, cabin(p_state)->get_controls(vehicle, cab));
        return 1;
    }

    /// occupied_cab(v) - 1, 0 (machine room) or -1
    static int cabin_occupied_cab(lua_State *p_state) {
        const RID vehicle = LuaHandle::check(p_state, 1, ScriptHandleKind::VEHICLE);
        lua_pushinteger(p_state, cabin(p_state)->get_occupied_cab(vehicle));
        return 1;
    }

    /// on_control_changed(v, fn) - fn(cab, control_id, value) whenever a control of the
    /// vehicle's cabs changes
    static int cabin_on_control_changed(lua_State *p_state) {
        return LuaModules::subscribe(
                p_state, ScenarioScriptServer::SIGNAL_CABIN_CONTROL_CHANGED, ScriptHandleKind::VEHICLE);
    }

    const luaL_Reg LuaModules::CABIN[] = {
            {"act", cabin_act},
            {"control", cabin_control},
            {"controls", cabin_controls},
            {"occupied_cab", cabin_occupied_cab},
            {"on_control_changed", cabin_on_control_changed},
            {nullptr, nullptr},
    };
} // namespace godot
