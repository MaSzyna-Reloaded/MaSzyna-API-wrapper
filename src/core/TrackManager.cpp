#include "LegacyRailVehicle.hpp"
#include "TrackManager.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void VirtualTrack::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_rid"), &VirtualTrack::get_rid);
        ClassDB::bind_method(D_METHOD("get_track_id"), &VirtualTrack::get_track_id);
        ClassDB::bind_method(D_METHOD("get_length"), &VirtualTrack::get_length);
        ClassDB::bind_method(D_METHOD("get_start_point"), &VirtualTrack::get_start_point);
        ClassDB::bind_method(D_METHOD("get_axis"), &VirtualTrack::get_axis);
        ClassDB::bind_method(D_METHOD("get_world_position", "offset"), &VirtualTrack::get_world_position);

        ADD_PROPERTY(PropertyInfo(Variant::RID, "rid", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_READ_ONLY), "", "get_rid");
        ADD_PROPERTY(
                PropertyInfo(Variant::STRING, "track_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_READ_ONLY),
                "", "get_track_id");
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "length", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_READ_ONLY),
                "", "get_length");
        ADD_PROPERTY(
                PropertyInfo(Variant::VECTOR3, "start_point", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_READ_ONLY),
                "", "get_start_point");
    }

    void VirtualTrack::configure(
            const RID &p_track_rid, const String &p_track_id, const double p_length, const Vector3 &p_start_point,
            const double p_track_width) {
        track_rid = p_track_rid;
        track_id = p_track_id;
        length = p_length;
        start_point = p_start_point;

        shape.R = 0.0;
        shape.Len = p_length;
        shape.dHtrack = 0.0;
        shape.dHrail = 0.0;

        track_param.Width = p_track_width;
        track_param.friction = Maszyna::Steel2Steel_friction;
        track_param.CategoryFlag = 1;
        track_param.QualityFlag = 20;
        track_param.DamageFlag = 0;
        track_param.Velmax = 300.0;
    }

    RID VirtualTrack::get_rid() const {
        return track_rid;
    }

    String VirtualTrack::get_track_id() const {
        return track_id;
    }

    double VirtualTrack::get_length() const {
        return length;
    }

    Vector3 VirtualTrack::get_start_point() const {
        return start_point;
    }

    Vector3 VirtualTrack::get_axis() const {
        return Vector3(0.0, 0.0, -1.0);
    }

    Vector3 VirtualTrack::get_world_position(const double offset) const {
        return start_point + get_axis() * static_cast<float>(offset);
    }

    const Maszyna::TTrackShape &VirtualTrack::get_shape() const {
        return shape;
    }

    const Maszyna::TTrackParam &VirtualTrack::get_track_param() const {
        return track_param;
    }

    TrackManager::TrackManager() {
        tracks.set_description("VirtualTrack");
    }

    void TrackManager::_bind_methods() {
        ClassDB::bind_method(D_METHOD("add_track", "length", "start_point", "track_id"), &TrackManager::add_track);
        ClassDB::bind_method(
                D_METHOD("modify_track", "track_rid", "length", "start_point", "track_id"), &TrackManager::modify_track);
        ClassDB::bind_method(D_METHOD("remove_track", "track_rid"), &TrackManager::remove_track);
        ClassDB::bind_method(D_METHOD("has_track", "track_id"), &TrackManager::has_track);
        ClassDB::bind_method(D_METHOD("get_track", "track_rid"), &TrackManager::get_track);
        ClassDB::bind_method(D_METHOD("get_track_by_name", "track_id"), &TrackManager::get_track_by_name);
        ClassDB::bind_method(
                D_METHOD("get_track_position", "track_rid", "offset"), &TrackManager::get_track_position);
        ClassDB::bind_method(
                D_METHOD("register_vehicle", "vehicle", "track_rid", "track_id", "offset"), &TrackManager::register_vehicle);
        ClassDB::bind_method(D_METHOD("update_vehicle_offset", "vehicle", "offset"), &TrackManager::update_vehicle_offset);
        ClassDB::bind_method(D_METHOD("remove_vehicle", "vehicle"), &TrackManager::remove_vehicle);
    }

    TrackManager::~TrackManager() {
        List<RID> owned_tracks;
        tracks.get_owned_list(&owned_tracks);
        for (const RID &track_rid : owned_tracks) {
            tracks.free(track_rid);
        }

        named_tracks.clear();
        vehicle_placements.clear();
    }

    void TrackManager::_report_duplicate_track_name(
            const String &track_id, const RID &existing_rid, const RID &requested_rid) const {
        const String message = "TrackManager: duplicate track_id '" + track_id + "' for RID " +
                String::num_uint64(requested_rid.get_id()) + ". Existing mapping keeps RID " +
                String::num_uint64(existing_rid.get_id()) + ".";
        UtilityFunctions::push_warning(message);
        UtilityFunctions::push_error(message);
    }

    RID TrackManager::add_track(const double length, const Vector3 &start_point, const String &track_id) {
        Ref<VirtualTrack> track;
        track.instantiate();

        const RID track_rid = tracks.make_rid(track);
        track->configure(track_rid, track_id, length, start_point, 1.435);

        if (!track_id.is_empty()) {
            const auto existing = named_tracks.find(track_id);
            if (existing != named_tracks.end() && existing->second != track_rid) {
                _report_duplicate_track_name(track_id, existing->second, track_rid);
            } else {
                named_tracks[track_id] = track_rid;
            }
        }

        return track_rid;
    }

    void TrackManager::modify_track(
            const RID &track_rid, const double length, const Vector3 &start_point, const String &track_id) {
        Ref<VirtualTrack> *track = tracks.get_or_null(track_rid);
        if (track == nullptr || track->is_null()) {
            return;
        }

        String effective_track_id = track_id;
        const String previous_track_id = (*track)->get_track_id();

        if (effective_track_id != previous_track_id) {
            if (!effective_track_id.is_empty()) {
                const auto existing = named_tracks.find(effective_track_id);
                if (existing != named_tracks.end() && existing->second != track_rid) {
                    _report_duplicate_track_name(effective_track_id, existing->second, track_rid);
                    effective_track_id = previous_track_id;
                }
            }

            if (effective_track_id != previous_track_id) {
                if (!previous_track_id.is_empty()) {
                    const auto previous = named_tracks.find(previous_track_id);
                    if (previous != named_tracks.end() && previous->second == track_rid) {
                        named_tracks.erase(previous);
                    }
                }

                if (!effective_track_id.is_empty()) {
                    named_tracks[effective_track_id] = track_rid;
                }
            }
        }

        (*track)->configure(track_rid, effective_track_id, length, start_point, 1.435);
    }

    void TrackManager::remove_track(const RID &track_rid) {
        Ref<VirtualTrack> *track = tracks.get_or_null(track_rid);
        if (track == nullptr || track->is_null()) {
            return;
        }

        const String track_id = (*track)->get_track_id();
        if (!track_id.is_empty()) {
            const auto named = named_tracks.find(track_id);
            if (named != named_tracks.end() && named->second == track_rid) {
                named_tracks.erase(named);
            }
        }

        for (auto it = vehicle_placements.begin(); it != vehicle_placements.end();) {
            if (it->second.track_rid == track_rid) {
                it = vehicle_placements.erase(it);
            } else {
                ++it;
            }
        }

        tracks.free(track_rid);
    }

    bool TrackManager::has_track(const String &track_id) const {
        return named_tracks.find(track_id) != named_tracks.end();
    }

    Ref<VirtualTrack> TrackManager::get_track(const RID &track_rid) const {
        const Ref<VirtualTrack> *track = tracks.get_or_null(track_rid);
        if (track == nullptr) {
            return Ref<VirtualTrack>();
        }

        return *track;
    }

    Ref<VirtualTrack> TrackManager::get_track_by_name(const String &track_id) const {
        const auto it = named_tracks.find(track_id);
        if (it == named_tracks.end()) {
            return Ref<VirtualTrack>();
        }

        return get_track(it->second);
    }

    Vector3 TrackManager::get_track_position(const RID &track_rid, const double offset) const {
        const Ref<VirtualTrack> track = get_track(track_rid);
        if (track.is_null()) {
            return Vector3();
        }

        return track->get_world_position(offset);
    }

    void TrackManager::register_vehicle(
            LegacyRailVehicle *vehicle, const RID &track_rid, const String &track_id, const double offset) {
        if (vehicle == nullptr) {
            return;
        }

        if (get_track(track_rid).is_null()) {
            UtilityFunctions::push_warning(
                    "TrackManager::register_vehicle(): unknown track RID " + String::num_uint64(track_rid.get_id()) + ".");
            return;
        }

        vehicle_placements[vehicle] = VehiclePlacement{track_rid, track_id, offset};
    }

    void TrackManager::update_vehicle_offset(LegacyRailVehicle *vehicle, const double offset) {
        if (vehicle == nullptr) {
            return;
        }

        const auto it = vehicle_placements.find(vehicle);
        if (it == vehicle_placements.end()) {
            return;
        }

        it->second.offset = offset;
    }

    void TrackManager::remove_vehicle(LegacyRailVehicle *vehicle) {
        if (vehicle == nullptr) {
            return;
        }

        vehicle_placements.erase(vehicle);
    }

    Ref<VirtualTrack> TrackManager::get_vehicle_track(LegacyRailVehicle *vehicle) const {
        if (vehicle == nullptr) {
            return Ref<VirtualTrack>();
        }

        const auto it = vehicle_placements.find(vehicle);
        if (it == vehicle_placements.end()) {
            return Ref<VirtualTrack>();
        }

        return get_track(it->second.track_rid);
    }

    bool TrackManager::get_vehicle_offset(LegacyRailVehicle *vehicle, double &offset) const {
        if (vehicle == nullptr) {
            return false;
        }

        const auto it = vehicle_placements.find(vehicle);
        if (it == vehicle_placements.end()) {
            return false;
        }

        offset = it->second.offset;
        return true;
    }

    LegacyRailVehicle *TrackManager::find_nearest_vehicle(
            LegacyRailVehicle *vehicle, const bool ahead, double *distance, int *other_end) const {
        if (vehicle == nullptr) {
            return nullptr;
        }

        const auto vehicle_it = vehicle_placements.find(vehicle);
        if (vehicle_it == vehicle_placements.end()) {
            return nullptr;
        }

        LegacyRailVehicle *nearest = nullptr;
        double nearest_distance = 0.0;

        for (const auto &[other_vehicle, placement] : vehicle_placements) {
            if (other_vehicle == vehicle || placement.track_rid != vehicle_it->second.track_rid) {
                continue;
            }

            const double delta = placement.offset - vehicle_it->second.offset;
            const bool candidate_matches = ahead ? (delta > 0.0) : (delta < 0.0);
            if (!candidate_matches) {
                continue;
            }

            const double candidate_distance = Math::abs(delta);
            if (nearest == nullptr || candidate_distance < nearest_distance) {
                nearest = other_vehicle;
                nearest_distance = candidate_distance;
            }
        }

        if (nearest != nullptr) {
            if (distance != nullptr) {
                *distance = nearest_distance;
            }
            if (other_end != nullptr) {
                *other_end = ahead ? Maszyna::end::rear : Maszyna::end::front;
            }
        }

        return nearest;
    }

    LegacyRailVehicle *TrackManager::get_nearest_vehicle_ahead(
            LegacyRailVehicle *vehicle, double *distance, int *other_end) const {
        return find_nearest_vehicle(vehicle, true, distance, other_end);
    }

    LegacyRailVehicle *TrackManager::get_nearest_vehicle_behind(
            LegacyRailVehicle *vehicle, double *distance, int *other_end) const {
        return find_nearest_vehicle(vehicle, false, distance, other_end);
    }
} // namespace godot
