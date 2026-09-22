#include "VehicleState.hpp"

#include "RailVehicleServer.hpp"
#include "VehiclePropertyRegistry.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void VehicleState::_bind_methods() {
        ClassDB::bind_static_method("VehicleState", D_METHOD("resolve", "name"), &VehicleState::resolve);
        ClassDB::bind_method(D_METHOD("get_by_id", "property_id"), &VehicleState::get_by_id);
        ClassDB::bind_method(D_METHOD("get_float_by_id", "property_id"), &VehicleState::get_float_by_id);
        ClassDB::bind_method(D_METHOD("read_floats", "property_ids"), &VehicleState::read_floats);
        ClassDB::bind_method(D_METHOD("snapshot"), &VehicleState::snapshot);
        ClassDB::bind_method(D_METHOD("is_valid"), &VehicleState::is_valid);
        ClassDB::bind_method(D_METHOD("get_vehicle"), &VehicleState::get_vehicle);
    }

    void VehicleState::set_vehicle(const RID &p_vehicle) {
        vehicle = p_vehicle;
    }

    RID VehicleState::get_vehicle() const {
        return vehicle;
    }

    bool VehicleState::is_valid() const {
        const RailVehicleServer *server = RailVehicleServer::get_instance();
        return server != nullptr && server->vehicle_exists(vehicle);
    }

    int VehicleState::resolve(const StringName &p_name) {
        return VehiclePropertyRegistry::get_id(p_name);
    }

    Variant VehicleState::get_by_id(const int p_property_id) const {
        const RailVehicleServer *server = RailVehicleServer::get_instance();
        if (server == nullptr) {
            return Variant();
        }
        return server->vehicle_get_state_value(vehicle, p_property_id);
    }

    double VehicleState::get_float_by_id(const int p_property_id) const {
        const Variant value = get_by_id(p_property_id);
        return value.get_type() == Variant::NIL ? 0.0 : static_cast<double>(value);
    }

    PackedFloat64Array VehicleState::read_floats(const PackedInt32Array &p_property_ids) const {
        PackedFloat64Array result;
        const int count = static_cast<int>(p_property_ids.size());
        result.resize(count);
        for (int index = 0; index < count; ++index) {
            result.set(index, get_float_by_id(p_property_ids[index]));
        }
        return result;
    }

    Dictionary VehicleState::snapshot() const {
        Dictionary result;
        const RailVehicleServer *server = RailVehicleServer::get_instance();
        if (server == nullptr) {
            return result;
        }
        const PackedInt32Array ids = server->vehicle_get_state_property_ids(vehicle);
        for (int index = 0; index < ids.size(); ++index) {
            const int property_id = ids[index];
            result[VehiclePropertyRegistry::get_descriptor(property_id).name] = get_by_id(property_id);
        }
        return result;
    }

    bool VehicleState::_get(const StringName &p_name, Variant &p_ret) const {
        const int property_id = VehiclePropertyRegistry::get_id(p_name);
        if (property_id < 0) {
            return false;
        }
        p_ret = get_by_id(property_id);
        return true;
    }

    /* State has an owner and changes through a named operation of that owner - a command. Writing
     * it from outside is what CODE_STYLE.md's "one writer per field" forbids, so this refuses and
     * says who to ask instead. */
    bool VehicleState::_set(const StringName &p_name, const Variant &p_value) {
        const int property_id = VehiclePropertyRegistry::get_id(p_name);
        if (property_id < 0) {
            return false;
        }
        ERR_FAIL_V_MSG(
                false,
                vformat("Vehicle property '%s' is owned by %s and changes through a command, not by assignment.",
                        p_name, VehiclePropertyRegistry::get_descriptor(property_id).owner));
    }

    /* Only what this vehicle actually publishes, so a console or the editor's Remote tree shows
     * the truth about this one rather than every property any vehicle could have. */
    void VehicleState::_get_property_list(List<PropertyInfo> *p_list) const {
        const RailVehicleServer *server = RailVehicleServer::get_instance();
        if (server == nullptr) {
            return;
        }
        const PackedInt32Array ids = server->vehicle_get_state_property_ids(vehicle);
        for (int index = 0; index < ids.size(); ++index) {
            const VehiclePropertyDescriptor &descriptor = VehiclePropertyRegistry::get_descriptor(ids[index]);
            p_list->push_back(PropertyInfo(
                    descriptor.type, descriptor.name, PROPERTY_HINT_NONE, "",
                    PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));
        }
    }
} // namespace godot
