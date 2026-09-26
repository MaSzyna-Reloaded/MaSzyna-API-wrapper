#include "../physics/RailVehicleServer.hpp"
#include "MaszynaLegacyVehicleCommandAction.hpp"
#include "ScenarioEventServer.hpp"

namespace godot {
    void MaszynaLegacyVehicleCommandAction::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_command", "command"), &MaszynaLegacyVehicleCommandAction::set_command);
        ClassDB::bind_method(D_METHOD("get_command"), &MaszynaLegacyVehicleCommandAction::get_command);
        ClassDB::bind_method(D_METHOD("set_value1", "value1"), &MaszynaLegacyVehicleCommandAction::set_value1);
        ClassDB::bind_method(D_METHOD("get_value1"), &MaszynaLegacyVehicleCommandAction::get_value1);
        ClassDB::bind_method(D_METHOD("set_source", "source"), &MaszynaLegacyVehicleCommandAction::set_source);
        ClassDB::bind_method(D_METHOD("get_source"), &MaszynaLegacyVehicleCommandAction::get_source);

        ADD_PROPERTY(PropertyInfo(Variant::STRING, "command"), "set_command", "get_command");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "value1"), "set_value1", "get_value1");
        ADD_PROPERTY(PropertyInfo(Variant::RID, "source"), "set_source", "get_source");
    }

    /// getvalues_event::run_() / putvalues_event::run_() (Event.cpp:615-635, 771-844): nothing
    /// without a vehicle to send it to
    void MaszynaLegacyVehicleCommandAction::run(const RID &p_event, const RID &p_activator) {
        if (!p_activator.is_valid()) {
            return;
        }
        const ScenarioEventServer *events = ScenarioEventServer::get_instance();
        RailVehicleServer *vehicles = RailVehicleServer::get_instance();
        ERR_FAIL_NULL(events);
        ERR_FAIL_NULL(vehicles);
        const bool read = source.is_valid();
        const String text = read ? events->memory_get_text(source) : command;
        if (text == CAB_SIGNAL) {
            vehicles->vehicle_send_command(p_activator, "security_cabsignal_trigger");
        } else if (text == EMERGENCY_BRAKE) {
            const double value = read ? events->memory_get_value1(source) : value1;
            vehicles->vehicle_send_command(p_activator, "security_radiostop", Math::floor(value) == 1.0);
        }
    }

    void MaszynaLegacyVehicleCommandAction::set_command(const String &p_command) {
        command = p_command;
    }

    String MaszynaLegacyVehicleCommandAction::get_command() const {
        return command;
    }

    void MaszynaLegacyVehicleCommandAction::set_value1(const double p_value1) {
        value1 = p_value1;
    }

    double MaszynaLegacyVehicleCommandAction::get_value1() const {
        return value1;
    }

    void MaszynaLegacyVehicleCommandAction::set_source(const RID &p_source) {
        source = p_source;
    }

    RID MaszynaLegacyVehicleCommandAction::get_source() const {
        return source;
    }
} // namespace godot
