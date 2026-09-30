#include "PlayerServer.hpp"
#include "driver/DriverSystem.hpp"
#include "vehicles/base/VehicleServer.hpp"
#include "vehicles/rail/RailVehicleRenderingServer.hpp"
#include "vehicles/rail/RailVehicleServer.hpp"
#include <godot_cpp/core/class_db.hpp>

namespace godot {
    const char *PlayerServer::player_vehicle_changed_signal = "player_vehicle_changed";
    const char *PlayerServer::player_vehicle_entered_signal = "player_vehicle_entered";

    /// The vehicle's trainset driven by its drivers again, or not
    static void set_trainset_control_active(const RID &p_vehicle, const bool p_active) {
        const RailVehicleServer *vehicles = RailVehicleServer::get_instance();
        DriverSystem *drivers = DriverSystem::get_instance();
        ERR_FAIL_NULL(vehicles);
        ERR_FAIL_NULL(drivers);
        const TypedArray<RID> trainset = vehicles->vehicle_get_coupled(
                p_vehicle, RailVehicleController::COUPLER_END_FRONT, RailVehicleController::COUPLING_FLAG_COUPLER);
        for (int index = 0; index < trainset.size(); ++index) {
            drivers->vehicle_set_control_active(trainset[index], p_active);
        }
    }

    void PlayerServer::_bind_methods() {
        ClassDB::bind_method(D_METHOD("player_enter_vehicle", "vehicle"), &PlayerServer::player_enter_vehicle);
        ClassDB::bind_method(D_METHOD("player_leave_vehicle"), &PlayerServer::player_leave_vehicle);
        ClassDB::bind_method(D_METHOD("player_get_vehicle"), &PlayerServer::player_get_vehicle);

        ADD_SIGNAL(MethodInfo(
                player_vehicle_changed_signal, PropertyInfo(Variant::RID, "vehicle"),
                PropertyInfo(Variant::RID, "previous")));
        ADD_SIGNAL(MethodInfo(player_vehicle_entered_signal, PropertyInfo(Variant::RID, "vehicle")));
    }

    /// A vehicle freed with the player in it leaves the player on foot. No explicit disconnect:
    /// callable_mp reports this instance as the callable's object, so the engine drops the
    /// connection when it dies.
    PlayerServer::PlayerServer() {
        VehicleServer *vehicles = VehicleServer::get_instance();
        ERR_FAIL_NULL(vehicles);
        vehicles->connect(VehicleServer::vehicle_freed_signal, callable_mp(this, &PlayerServer::_on_vehicle_freed));
    }

    void PlayerServer::_on_vehicle_freed(const RID &p_vehicle) {
        if (p_vehicle == vehicle) {
            _set_vehicle(RID());
        }
    }

    void PlayerServer::_set_vehicle(const RID &p_vehicle) {
        const RID previous = vehicle;
        vehicle = p_vehicle;
        emit_signal(player_vehicle_changed_signal, vehicle, previous);
    }

    void PlayerServer::player_enter_vehicle(const RID &p_vehicle) {
        DriverSystem *drivers = DriverSystem::get_instance();
        ERR_FAIL_NULL(drivers);
        if (p_vehicle == vehicle && vehicle.is_valid()) {
            // the train already driven: only the driver switched off (aidriverdisable), and the
            // player back in its cab (InOutKey(), drivermode.cpp:258-267)
            drivers->vehicle_set_control_active(p_vehicle, false);
            emit_signal(player_vehicle_entered_signal, p_vehicle);
            return;
        }
        VehicleServer *vehicle_server = VehicleServer::get_instance();
        RailVehicleServer *vehicles = RailVehicleServer::get_instance();
        ERR_FAIL_NULL(vehicle_server);
        ERR_FAIL_NULL(vehicles);
        ERR_FAIL_COND(!vehicle_server->vehicle_exists(p_vehicle));
        const RailVehicleRenderingServer *drawn = RailVehicleRenderingServer::get_instance();
        ERR_FAIL_COND_MSG(
                drawn == nullptr || !drawn->vehicle_is_attached(p_vehicle), "The vehicle has no cab to sit in");
        if (vehicle.is_valid()) {
            // another trainset's vehicle: the one left is driven by its drivers again
            const TypedArray<RID> trainset = vehicles->vehicle_get_coupled(
                    vehicle, RailVehicleController::COUPLER_END_FRONT, RailVehicleController::COUPLING_FLAG_COUPLER);
            if (!trainset.has(p_vehicle)) {
                set_trainset_control_active(vehicle, true);
            }
        }
        drivers->vehicle_set_control_active(p_vehicle, false);
        // taking a vehicle over activates its cab when the FIZ allows it (Train.cpp:9147)
        vehicle_server->vehicle_send_command(p_vehicle, "cab_activation_auto");
        _set_vehicle(p_vehicle);
        emit_signal(player_vehicle_entered_signal, p_vehicle);
    }

    void PlayerServer::player_leave_vehicle() {
        if (!vehicle.is_valid()) {
            return;
        }
        set_trainset_control_active(vehicle, true);
        _set_vehicle(RID());
    }

    RID PlayerServer::player_get_vehicle() const {
        return vehicle;
    }
} // namespace godot
