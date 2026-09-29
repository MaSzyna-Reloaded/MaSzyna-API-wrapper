#include "PlayerServer.hpp"
#include "driver/DriverSystem.hpp"
#include "vehicles/rail/RailVehicleServer.hpp"
#include <godot_cpp/core/class_db.hpp>

namespace godot {
    const char *PlayerServer::player_vehicle_changed_signal = "player_vehicle_changed";

    namespace {
        /// The end vehicle_get_coupled() starts listing a trainset from
        constexpr int FRONT_END = 0;
    } // namespace

    /// The vehicle's trainset driven by its drivers again, or not
    static void set_trainset_control_active(const RID &p_vehicle, const bool p_active) {
        const RailVehicleServer *vehicles = RailVehicleServer::get_instance();
        DriverSystem *drivers = DriverSystem::get_instance();
        ERR_FAIL_NULL(vehicles);
        ERR_FAIL_NULL(drivers);
        const TypedArray<RID> trainset =
                vehicles->vehicle_get_coupled(p_vehicle, FRONT_END, RailVehicleController::COUPLING_ELEMENT_COUPLER);
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
    }

    /// A vehicle freed with the player in it leaves the player on foot. No explicit disconnect:
    /// callable_mp reports this instance as the callable's object, so the engine drops the
    /// connection when it dies.
    PlayerServer::PlayerServer() {
        RailVehicleServer *vehicles = RailVehicleServer::get_instance();
        ERR_FAIL_NULL(vehicles);
        vehicles->connect(RailVehicleServer::vehicle_freed_signal, callable_mp(this, &PlayerServer::_on_vehicle_freed));
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
        if (p_vehicle == vehicle) {
            return;
        }
        RailVehicleServer *vehicles = RailVehicleServer::get_instance();
        DriverSystem *drivers = DriverSystem::get_instance();
        ERR_FAIL_NULL(vehicles);
        ERR_FAIL_NULL(drivers);
        ERR_FAIL_COND(!vehicles->vehicle_exists(p_vehicle));
        ERR_FAIL_COND_MSG(vehicles->vehicle_get_rail_vehicle(p_vehicle) == 0, "The vehicle has no cab to sit in");
        if (vehicle.is_valid()) {
            // another trainset's vehicle: the one left is driven by its drivers again
            const TypedArray<RID> trainset =
                    vehicles->vehicle_get_coupled(vehicle, FRONT_END, RailVehicleController::COUPLING_ELEMENT_COUPLER);
            if (!trainset.has(p_vehicle)) {
                set_trainset_control_active(vehicle, true);
            }
        }
        drivers->vehicle_set_control_active(p_vehicle, false);
        // taking a vehicle over activates its cab when the FIZ allows it (Train.cpp:9147)
        vehicles->vehicle_send_command(p_vehicle, "cab_activation_auto");
        _set_vehicle(p_vehicle);
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
