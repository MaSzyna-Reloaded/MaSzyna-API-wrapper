#include "MaszynaMoverPhysicsServer.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void MaszynaMoverPhysicsServer::_bind_methods() {}

    MaszynaMoverPhysicsServer::~MaszynaMoverPhysicsServer() {
        for (KeyValue<RID, VehicleRecord> &item: vehicles) {
            delete item.value.mover;
        }
        vehicles.clear();
    }

    RID MaszynaMoverPhysicsServer::vehicle_create(
            const String &p_type_name, const String &p_name, const double p_initial_velocity,
            const int p_cabin_number) {
        ++next_vehicle_id;
        const RID vehicle_rid = UtilityFunctions::rid_from_int64(next_vehicle_id);
        VehicleRecord record;
        record.mover = new TMoverParameters(
                p_initial_velocity, std::string(p_type_name.utf8().get_data()),
                std::string(p_name.utf8().get_data()), p_cabin_number);
        vehicles.insert(vehicle_rid, record);
        return vehicle_rid;
    }

    void MaszynaMoverPhysicsServer::vehicle_free(const RID &p_vehicle) {
        VehicleRecord *record = vehicles.getptr(p_vehicle);
        if (record == nullptr) {
            return;
        }
        delete record->mover;
        vehicles.erase(p_vehicle);
    }

    bool MaszynaMoverPhysicsServer::vehicle_exists(const RID &p_vehicle) const {
        return vehicles.has(p_vehicle);
    }

    bool MaszynaMoverPhysicsServer::vehicle_is_active(const RID &p_vehicle) const {
        const VehicleRecord *record = vehicles.getptr(p_vehicle);
        return record != nullptr && record->mover->PhysicActivation;
    }

    double MaszynaMoverPhysicsServer::vehicle_get_velocity(const RID &p_vehicle) const {
        const VehicleRecord *record = vehicles.getptr(p_vehicle);
        return record != nullptr ? record->mover->V : 0.0;
    }

    double MaszynaMoverPhysicsServer::vehicle_get_speed(const RID &p_vehicle) const {
        const VehicleRecord *record = vehicles.getptr(p_vehicle);
        return record != nullptr ? record->mover->Vel : 0.0;
    }

    void MaszynaMoverPhysicsServer::vehicle_set_location(const RID &p_vehicle, const Vector3 &p_position) {
        const VehicleRecord *record = vehicles.getptr(p_vehicle);
        if (record == nullptr) {
            return;
        }
        record->mover->Loc = {-p_position.x, p_position.z, p_position.y};
        record->mover->dMoveLen = 0.0;
    }

    void MaszynaMoverPhysicsServer::vehicle_compute_forces(const RID &p_vehicle, const double p_delta) {
        const VehicleRecord *record = vehicles.getptr(p_vehicle);
        if (record == nullptr) {
            return;
        }
        record->mover->ComputeTotalForce(p_delta);
    }

    double MaszynaMoverPhysicsServer::vehicle_compute_movement(
            const RID &p_vehicle, const double p_delta, const MovementKind p_kind) {
        const VehicleRecord *record = vehicles.getptr(p_vehicle);
        // a standing vehicle switched off by ComputeTotalForce() is not moved at all
        // (DynObj.cpp:4059 FastUpdate, DynObj.cpp:2940 Update)
        if (record == nullptr || !record->mover->PhysicActivation) {
            return 0.0;
        }
        TMoverParameters *mover = record->mover;
        TRotation rotation;
        if (p_kind == MOVEMENT_FULL) {
            mover->ComputeMovement(
                    p_delta, p_delta, mover->RunningShape, mover->RunningTrack, mover->RunningTraction, mover->Loc,
                    rotation);
        } else {
            mover->FastComputeMovement(p_delta, mover->RunningShape, mover->RunningTrack, mover->Loc, rotation);
        }
        // the vehicle is moved by this distance (DynObj.cpp:2439), front-relative
        const double distance = mover->V * p_delta;
        mover->dMoveLen += distance;
        return distance;
    }

    TMoverParameters *MaszynaMoverPhysicsServer::vehicle_get_mover(const RID &p_vehicle) const {
        const VehicleRecord *record = vehicles.getptr(p_vehicle);
        return record != nullptr ? record->mover : nullptr;
    }

} // namespace godot
