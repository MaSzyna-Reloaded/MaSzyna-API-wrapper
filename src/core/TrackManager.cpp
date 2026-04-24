#include "TrackManager.hpp"
#include "TrainController.hpp"
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>
#include <cmath>

namespace godot {
    namespace {
        constexpr double DEFAULT_TRACK_WIDTH = 1.435;
        constexpr int MAX_TRACK_TRANSITIONS = 64;
        constexpr double DEFAULT_TRACK_FRICTION = 0.15;
        constexpr int TERMINAL_TRACK_DAMAGE_FLAG = 128;

        double compute_radius(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2) {
            const double a = p0.distance_to(p1);
            const double b = p1.distance_to(p2);
            const double c = p2.distance_to(p0);
            if (a <= 1e-6 || b <= 1e-6 || c <= 1e-6) {
                return 0.0;
            }

            const double semiperimeter = 0.5 * (a + b + c);
            const double area_sq = semiperimeter * (semiperimeter - a) * (semiperimeter - b) * (semiperimeter - c);
            if (area_sq <= 1e-6) {
                return 0.0;
            }

            const double radius = (a * b * c) / (4.0 * std::sqrt(area_sq));
            if (!std::isfinite(radius)) {
                return 0.0;
            }

            const Vector3 cross = (p1 - p0).cross(p2 - p1);
            const double sign = cross.y >= 0.0 ? 1.0 : -1.0;
            return radius * sign;
        }
    } // namespace

    void VirtualTrack::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_rid"), &VirtualTrack::get_rid);
        ClassDB::bind_method(D_METHOD("get_track_id"), &VirtualTrack::get_track_id);
        ClassDB::bind_method(D_METHOD("get_length"), &VirtualTrack::get_length);
        ClassDB::bind_method(D_METHOD("get_start_point"), &VirtualTrack::get_start_point);
        ClassDB::bind_method(D_METHOD("get_previous_track_rid"), &VirtualTrack::get_previous_track_rid);
        ClassDB::bind_method(D_METHOD("get_next_track_rid"), &VirtualTrack::get_next_track_rid);
        ClassDB::bind_method(D_METHOD("get_axis", "offset"), &VirtualTrack::get_axis, DEFVAL(0.0));
        ClassDB::bind_method(D_METHOD("get_world_position", "offset"), &VirtualTrack::get_world_position);

        ADD_PROPERTY(PropertyInfo(Variant::RID, "rid", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_READ_ONLY), "", "get_rid");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "track_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_READ_ONLY), "", "get_track_id");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "length", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_READ_ONLY), "", "get_length");
        ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "start_point", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_READ_ONLY), "", "get_start_point");
    }

    void VirtualTrack::configure(
            const RID &p_track_rid, const String &p_track_id, const double p_length, const Vector3 &p_start_point,
            const double p_track_width) {
        PackedVector3Array points;
        points.push_back(p_start_point);
        points.push_back(p_start_point + Vector3(0.0, 0.0, -1.0) * static_cast<float>(p_length));
        configure_path(p_track_rid, p_track_id, points, p_track_width);
    }

    void VirtualTrack::configure_path(
            const RID &p_track_rid, const String &p_track_id, const PackedVector3Array &p_points,
            const double p_track_width, const RID &p_previous_track_rid, const RID &p_next_track_rid) {
        track_rid = p_track_rid;
        track_id = p_track_id;
        previous_track_rid = p_previous_track_rid;
        next_track_rid = p_next_track_rid;
        sampled_points.clear();
        sampled_offsets.clear();

        if (p_points.is_empty()) {
            sampled_points.push_back(Vector3());
            sampled_offsets.push_back(0.0);
        } else {
            sampled_points.reserve(p_points.size());
            sampled_offsets.reserve(p_points.size());

            double accumulated_length = 0.0;
            Vector3 previous_point = p_points[0];
            sampled_points.push_back(previous_point);
            sampled_offsets.push_back(0.0);

            for (int index = 1; index < p_points.size(); ++index) {
                const Vector3 point = p_points[index];
                accumulated_length += previous_point.distance_to(point);
                sampled_points.push_back(point);
                sampled_offsets.push_back(accumulated_length);
                previous_point = point;
            }
        }

        start_point = sampled_points.front();
        length = sampled_offsets.back();

        shape.radius = 0.0;
        shape.length = length;
        shape.track_height_delta = 0.0;
        shape.rail_height_delta = 0.0;

        runtime_profile.width = p_track_width;
        runtime_profile.friction = DEFAULT_TRACK_FRICTION;
        runtime_profile.category_flags = 1;
        runtime_profile.quality_flags = 20;
        runtime_profile.damage_flags = 0;
        runtime_profile.velocity_limit = 300.0;
    }

    RID VirtualTrack::get_rid() const { return track_rid; }
    String VirtualTrack::get_track_id() const { return track_id; }
    double VirtualTrack::get_length() const { return length; }
    Vector3 VirtualTrack::get_start_point() const { return start_point; }
    RID VirtualTrack::get_previous_track_rid() const { return previous_track_rid; }
    RID VirtualTrack::get_next_track_rid() const { return next_track_rid; }

    double VirtualTrack::clamp_offset(const double offset) const {
        return offset;
    }

    bool VirtualTrack::sample_position(const double offset, Vector3 &out_position) const {
        if (sampled_points.empty()) {
            out_position = Vector3();
            return false;
        }

        if (sampled_points.size() == 1 || length <= 0.0) {
            out_position = sampled_points.front();
            return true;
        }

        if (offset <= sampled_offsets.front()) {
            const Vector3 direction = (sampled_points[1] - sampled_points[0]).normalized();
            out_position = sampled_points[0] + direction * static_cast<float>(offset - sampled_offsets.front());
            return true;
        }

        if (offset >= sampled_offsets.back()) {
            const int last_index = static_cast<int>(sampled_points.size()) - 1;
            const Vector3 direction = (sampled_points[last_index] - sampled_points[last_index - 1]).normalized();
            out_position = sampled_points[last_index] + direction * static_cast<float>(offset - sampled_offsets.back());
            return true;
        }

        const auto upper = std::lower_bound(sampled_offsets.begin(), sampled_offsets.end(), offset);
        const int upper_index = static_cast<int>(upper - sampled_offsets.begin());

        const int lower_index = upper_index - 1;
        const double lower_offset = sampled_offsets[lower_index];
        const double upper_offset = sampled_offsets[upper_index];
        const double segment_length = upper_offset - lower_offset;
        if (segment_length <= 1e-6) {
            out_position = sampled_points[upper_index];
            return true;
        }

        const double weight = (offset - lower_offset) / segment_length;
        out_position = sampled_points[lower_index].lerp(sampled_points[upper_index], static_cast<float>(weight));
        return true;
    }

    bool VirtualTrack::sample_axis(const double offset, Vector3 &out_axis) const {
        if (sampled_points.size() < 2) {
            out_axis = Vector3(0.0, 0.0, -1.0);
            return false;
        }

        if (length <= 0.0) {
            out_axis = (sampled_points.back() - sampled_points.front()).normalized();
            if (out_axis.is_zero_approx()) {
                out_axis = Vector3(0.0, 0.0, -1.0);
            }
            return true;
        }

        if (offset <= sampled_offsets.front()) {
            out_axis = (sampled_points[1] - sampled_points[0]).normalized();
            if (out_axis.is_zero_approx()) {
                out_axis = Vector3(0.0, 0.0, -1.0);
            }
            return true;
        }

        if (offset >= sampled_offsets.back()) {
            const int last_index = static_cast<int>(sampled_points.size()) - 1;
            out_axis = (sampled_points[last_index] - sampled_points[last_index - 1]).normalized();
            if (out_axis.is_zero_approx()) {
                out_axis = Vector3(0.0, 0.0, -1.0);
            }
            return true;
        }

        const auto upper = std::lower_bound(sampled_offsets.begin(), sampled_offsets.end(), offset);
        int segment_index = static_cast<int>(upper - sampled_offsets.begin()) - 1;
        segment_index = std::clamp(segment_index, 0, static_cast<int>(sampled_points.size()) - 2);

        out_axis = (sampled_points[segment_index + 1] - sampled_points[segment_index]).normalized();
        if (out_axis.is_zero_approx()) {
            out_axis = Vector3(0.0, 0.0, -1.0);
        }
        return true;
    }

    bool VirtualTrack::sample_shape(const double offset, TrackGeometry &out_shape) const {
        out_shape = shape;
        if (sampled_points.size() < 3) {
            out_shape.radius = 0.0;
            return false;
        }

        if (offset <= sampled_offsets.front()) {
            out_shape.radius = compute_radius(sampled_points[0], sampled_points[1], sampled_points[2]);
            return true;
        }

        if (offset >= sampled_offsets.back()) {
            const int last_index = static_cast<int>(sampled_points.size()) - 1;
            out_shape.radius = compute_radius(
                    sampled_points[last_index - 2], sampled_points[last_index - 1], sampled_points[last_index]);
            return true;
        }

        const auto upper = std::lower_bound(sampled_offsets.begin(), sampled_offsets.end(), offset);
        int center_index = static_cast<int>(upper - sampled_offsets.begin());
        center_index = std::clamp(center_index, 1, static_cast<int>(sampled_points.size()) - 2);

        out_shape.radius = compute_radius(
                sampled_points[center_index - 1], sampled_points[center_index], sampled_points[center_index + 1]);
        out_shape.length = length;
        out_shape.track_height_delta = 0.0;
        out_shape.rail_height_delta = 0.0;
        return true;
    }

    Vector3 VirtualTrack::get_axis(const double offset) const {
        Vector3 axis;
        sample_axis(offset, axis);
        return axis;
    }

    Vector3 VirtualTrack::get_world_position(const double offset) const {
        Vector3 position;
        sample_position(offset, position);
        return position;
    }

    void VirtualTrack::set_runtime_profile(const TrackRuntimeProfile &p_runtime_profile) {
        runtime_profile = p_runtime_profile;
    }

    void VirtualTrack::set_heights(const double p_track_height_delta, const double p_rail_height_delta) {
        shape.track_height_delta = p_track_height_delta;
        shape.rail_height_delta = p_rail_height_delta;
    }

    const TrackGeometry &VirtualTrack::get_shape() const { return shape; }
    const TrackRuntimeProfile &VirtualTrack::get_runtime_profile() const { return runtime_profile; }

    TrackManager::TrackManager() {
        tracks.set_description("VirtualTrack");
    }

    void TrackManager::_bind_methods() {
        ClassDB::bind_method(D_METHOD("add_track", "length", "start_point", "track_id"), &TrackManager::add_track);
        ClassDB::bind_method(D_METHOD("modify_track", "track_rid", "length", "start_point", "track_id"), &TrackManager::modify_track);
        ClassDB::bind_method(
                D_METHOD("add_path_track", "points", "track_id", "previous_track_rid", "next_track_rid"),
                &TrackManager::add_path_track, DEFVAL(""), DEFVAL(RID()), DEFVAL(RID()));
        ClassDB::bind_method(
                D_METHOD("modify_path_track", "track_rid", "points", "track_id", "previous_track_rid", "next_track_rid"),
                &TrackManager::modify_path_track, DEFVAL(""), DEFVAL(RID()), DEFVAL(RID()));
        ClassDB::bind_method(D_METHOD("remove_track", "track_rid"), &TrackManager::remove_track);
        ClassDB::bind_method(D_METHOD("has_track", "track_id"), &TrackManager::has_track);
        ClassDB::bind_method(D_METHOD("get_track", "track_rid"), &TrackManager::get_track);
        ClassDB::bind_method(D_METHOD("get_track_by_name", "track_id"), &TrackManager::get_track_by_name);
        ClassDB::bind_method(D_METHOD("get_track_position", "track_rid", "offset"), &TrackManager::get_track_position);
        ClassDB::bind_method(D_METHOD("get_track_axis", "track_rid", "offset"), &TrackManager::get_track_axis);
        ClassDB::bind_method(D_METHOD("resolve_track_position", "track_rid", "offset"), &TrackManager::resolve_track_position);
        ClassDB::bind_method(D_METHOD("register_vehicle", "vehicle", "track_rid", "track_id", "offset"), &TrackManager::register_vehicle);
        ClassDB::bind_method(D_METHOD("update_vehicle_offset", "vehicle", "offset"), &TrackManager::update_vehicle_offset);
        ClassDB::bind_method(D_METHOD("set_track_heights", "track_rid", "track_height_delta", "rail_height_delta"), &TrackManager::set_track_heights);
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

    bool TrackManager::_update_track_name_mapping(
            const RID &track_rid, const String &previous_track_id, const String &track_id) {
        String effective_track_id = track_id;

        if (effective_track_id != previous_track_id) {
            if (!effective_track_id.is_empty()) {
                const auto existing = named_tracks.find(effective_track_id);
                if (existing != named_tracks.end() && existing->second != track_rid) {
                    _report_duplicate_track_name(effective_track_id, existing->second, track_rid);
                    return false;
                }
            }

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

        return true;
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
        track->configure(track_rid, track_id, length, start_point, DEFAULT_TRACK_WIDTH);
        _update_track_name_mapping(track_rid, "", track_id);
        return track_rid;
    }

    void TrackManager::modify_track(
            const RID &track_rid, const double length, const Vector3 &start_point, const String &track_id) {
        Ref<VirtualTrack> *track = tracks.get_or_null(track_rid);
        if (track == nullptr || track->is_null()) {
            return;
        }

        const String previous_track_id = (*track)->get_track_id();
        if (!_update_track_name_mapping(track_rid, previous_track_id, track_id)) {
            (*track)->configure(track_rid, previous_track_id, length, start_point, DEFAULT_TRACK_WIDTH);
            return;
        }

        (*track)->configure(track_rid, track_id, length, start_point, DEFAULT_TRACK_WIDTH);
    }

    RID TrackManager::add_path_track(
            const PackedVector3Array &points, const String &track_id, const RID &previous_track_rid,
            const RID &next_track_rid) {
        Ref<VirtualTrack> track;
        track.instantiate();

        const RID track_rid = tracks.make_rid(track);
        track->configure_path(track_rid, track_id, points, DEFAULT_TRACK_WIDTH, previous_track_rid, next_track_rid);
        _update_track_name_mapping(track_rid, "", track_id);
        return track_rid;
    }

    void TrackManager::modify_path_track(
            const RID &track_rid, const PackedVector3Array &points, const String &track_id,
            const RID &previous_track_rid, const RID &next_track_rid) {
        Ref<VirtualTrack> *track = tracks.get_or_null(track_rid);
        if (track == nullptr || track->is_null()) {
            return;
        }

        const String previous_track_id = (*track)->get_track_id();
        if (!_update_track_name_mapping(track_rid, previous_track_id, track_id)) {
            (*track)->configure_path(
                    track_rid, previous_track_id, points, DEFAULT_TRACK_WIDTH, previous_track_rid, next_track_rid);
            return;
        }

        (*track)->configure_path(track_rid, track_id, points, DEFAULT_TRACK_WIDTH, previous_track_rid, next_track_rid);
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

        for (auto it = terminal_tracks.begin(); it != terminal_tracks.end();) {
            if (it->first.source_track_id != track_rid.get_id()) {
                ++it;
                continue;
            }

            tracks.free(it->second);
            it = terminal_tracks.erase(it);
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

    bool TrackManager::resolve_track_position_internal(
            const RID &track_rid, double offset, RID &resolved_track_rid, String &resolved_track_id, double &resolved_offset,
            Vector3 *resolved_position, Vector3 *resolved_axis, TrackGeometry *resolved_shape) const {
        Ref<VirtualTrack> track = get_track(track_rid);
        if (track.is_null()) {
            return false;
        }

        resolved_track_rid = track_rid;
        resolved_track_id = track->get_track_id();
        resolved_offset = offset;

        for (int transitions = 0; transitions < MAX_TRACK_TRANSITIONS; ++transitions) {
            const double track_length = track->get_length();
            if (resolved_offset < 0.0 && track->get_previous_track_rid() != RID()) {
                Ref<VirtualTrack> previous_track = get_track(track->get_previous_track_rid());
                if (previous_track.is_null()) {
                    break;
                }
                resolved_offset += previous_track->get_length();
                track = previous_track;
                resolved_track_rid = track->get_rid();
                resolved_track_id = track->get_track_id();
                continue;
            }

            if (resolved_offset < 0.0) {
                Ref<VirtualTrack> terminal_track = get_track(get_or_create_terminal_track(track, false));
                if (terminal_track.is_null()) {
                    break;
                }

                resolved_offset = -resolved_offset;
                track = terminal_track;
                resolved_track_rid = track->get_rid();
                resolved_track_id = track->get_track_id();
                continue;
            }

            if (resolved_offset > track_length && track->get_next_track_rid() != RID()) {
                Ref<VirtualTrack> next_track = get_track(track->get_next_track_rid());
                if (next_track.is_null()) {
                    break;
                }
                resolved_offset -= track_length;
                track = next_track;
                resolved_track_rid = track->get_rid();
                resolved_track_id = track->get_track_id();
                continue;
            }

            if (resolved_offset > track_length) {
                Ref<VirtualTrack> terminal_track = get_track(get_or_create_terminal_track(track, true));
                if (terminal_track.is_null()) {
                    break;
                }

                resolved_offset -= track_length;
                track = terminal_track;
                resolved_track_rid = track->get_rid();
                resolved_track_id = track->get_track_id();
                continue;
            }

            if (resolved_position != nullptr) {
                track->sample_position(resolved_offset, *resolved_position);
            }
            if (resolved_axis != nullptr) {
                track->sample_axis(resolved_offset, *resolved_axis);
            }
            if (resolved_shape != nullptr) {
                track->sample_shape(resolved_offset, *resolved_shape);
            }
            return true;
        }

        if (resolved_position != nullptr) {
            track->sample_position(resolved_offset, *resolved_position);
        }
        if (resolved_axis != nullptr) {
            track->sample_axis(resolved_offset, *resolved_axis);
        }
        if (resolved_shape != nullptr) {
            track->sample_shape(resolved_offset, *resolved_shape);
        }
        resolved_track_rid = track->get_rid();
        resolved_track_id = track->get_track_id();
        return true;
    }

    Vector3 TrackManager::get_track_position(const RID &track_rid, const double offset) const {
        RID resolved_track_rid;
        String resolved_track_id;
        double resolved_offset = 0.0;
        Vector3 resolved_position;
        if (!resolve_track_position_internal(
                    track_rid, offset, resolved_track_rid, resolved_track_id, resolved_offset, &resolved_position)) {
            return Vector3();
        }

        return resolved_position;
    }

    Vector3 TrackManager::get_track_axis(const RID &track_rid, const double offset) const {
        RID resolved_track_rid;
        String resolved_track_id;
        double resolved_offset = 0.0;
        Vector3 resolved_axis;
        if (!resolve_track_position_internal(
                    track_rid, offset, resolved_track_rid, resolved_track_id, resolved_offset, nullptr, &resolved_axis)) {
            return Vector3(0.0, 0.0, -1.0);
        }

        return resolved_axis;
    }

    Dictionary TrackManager::resolve_track_position(const RID &track_rid, const double offset) const {
        Dictionary resolved;
        RID resolved_track_rid;
        String resolved_track_id;
        double resolved_offset = 0.0;
        Vector3 resolved_position;
        Vector3 resolved_axis;
        TrackGeometry resolved_shape;
        const bool valid = resolve_track_position_internal(
                track_rid, offset, resolved_track_rid, resolved_track_id, resolved_offset, &resolved_position, &resolved_axis, &resolved_shape);

        resolved["valid"] = valid;
        resolved["track_rid"] = resolved_track_rid;
        resolved["track_id"] = resolved_track_id;
        resolved["offset"] = resolved_offset;
        resolved["position"] = resolved_position;
        resolved["axis"] = resolved_axis;
        resolved["vertical_offset"] = resolved_shape.track_height_delta + resolved_shape.rail_height_delta;
        return resolved;
    }

    bool TrackManager::resolve_track_state(
            const RID &track_rid, const double offset, RID &resolved_track_rid, String &resolved_track_id,
            double &resolved_offset, Vector3 *resolved_position, Vector3 *resolved_axis, TrackGeometry *resolved_shape) const {
        return resolve_track_position_internal(
                track_rid, offset, resolved_track_rid, resolved_track_id, resolved_offset, resolved_position,
                resolved_axis, resolved_shape);
    }

    void TrackManager::register_vehicle(
            TrainController *vehicle, const RID &track_rid, const String &track_id, const double offset) {
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

    void TrackManager::update_vehicle_offset(TrainController *vehicle, const double offset) {
        if (vehicle == nullptr) {
            return;
        }

        const auto it = vehicle_placements.find(vehicle);
        if (it == vehicle_placements.end()) {
            return;
        }

        it->second.offset = offset;
    }

    void TrackManager::set_track_heights(const RID &track_rid, const double track_height_delta, const double rail_height_delta) {
        Ref<VirtualTrack> track = get_track(track_rid);
        if (track.is_valid()) {
            track->set_heights(track_height_delta, rail_height_delta);
        }
    }

    void TrackManager::remove_vehicle(TrainController *vehicle) {
        if (vehicle == nullptr) {
            return;
        }

        vehicle_placements.erase(vehicle);
    }

    Ref<VirtualTrack> TrackManager::get_vehicle_track(TrainController *vehicle) const {
        if (vehicle == nullptr) {
            return Ref<VirtualTrack>();
        }

        const auto it = vehicle_placements.find(vehicle);
        if (it == vehicle_placements.end()) {
            return Ref<VirtualTrack>();
        }

        return get_track(it->second.track_rid);
    }

    bool TrackManager::get_vehicle_offset(TrainController *vehicle, double &offset) const {
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

    TrainController *TrackManager::find_nearest_vehicle(
            TrainController *vehicle, const bool ahead, double *distance, int *other_end) const {
        if (vehicle == nullptr) {
            return nullptr;
        }

        const auto vehicle_it = vehicle_placements.find(vehicle);
        if (vehicle_it == vehicle_placements.end()) {
            return nullptr;
        }

        TrainController *nearest = nullptr;
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

    RID TrackManager::get_or_create_terminal_track(const Ref<VirtualTrack> &source_track, const bool from_end) const {
        if (source_track.is_null()) {
            return RID();
        }

        const TerminalTrackKey key{static_cast<uint64_t>(source_track->get_rid().get_id()), from_end};
        const auto existing = terminal_tracks.find(key);
        if (existing != terminal_tracks.end()) {
            return existing->second;
        }

        Ref<VirtualTrack> terminal_track;
        terminal_track.instantiate();

        const RID terminal_track_rid = tracks.make_rid(terminal_track);
        const Vector3 anchor = source_track->get_world_position(from_end ? source_track->get_length() : 0.0);
        Vector3 direction = source_track->get_axis(from_end ? source_track->get_length() : 0.0);
        if (!from_end) {
            direction = -direction;
        }
        if (direction.is_zero_approx()) {
            direction = Vector3(0.0, 0.0, -1.0);
        }

        PackedVector3Array terminal_points;
        terminal_points.push_back(anchor);
        terminal_points.push_back(anchor + direction.normalized() * 450.0);

        TrackRuntimeProfile terminal_profile = source_track->get_runtime_profile();
        terminal_profile.damage_flags |= TERMINAL_TRACK_DAMAGE_FLAG;
        terminal_profile.velocity_limit = 0.0;

        // Keep end-of-track handling inside the track layer by switching to a dedicated terminal segment.
        terminal_track->configure_path(terminal_track_rid, "", terminal_points, terminal_profile.width);
        terminal_track->set_runtime_profile(terminal_profile);

        terminal_tracks[key] = terminal_track_rid;
        return terminal_track_rid;
    }

    TrainController *TrackManager::get_nearest_vehicle_ahead(
            TrainController *vehicle, double *distance, int *other_end) const {
        return find_nearest_vehicle(vehicle, true, distance, other_end);
    }

    TrainController *TrackManager::get_nearest_vehicle_behind(
            TrainController *vehicle, double *distance, int *other_end) const {
        return find_nearest_vehicle(vehicle, false, distance, other_end);
    }
} // namespace godot
