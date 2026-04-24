#pragma once

#include "../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/rid_owner.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <map>
#include <vector>

namespace godot {
    class TrainController;

    class VirtualTrack : public RefCounted {
            GDCLASS(VirtualTrack, RefCounted)

        private:
            RID track_rid;
            String track_id;
            double length = 0.0;
            Vector3 start_point = Vector3();
            RID previous_track_rid;
            RID next_track_rid;
            std::vector<Vector3> sampled_points;
            std::vector<double> sampled_offsets;
            Maszyna::TTrackShape shape;
            Maszyna::TTrackParam track_param;

        protected:
            static void _bind_methods();

        public:
            VirtualTrack() = default;

            void configure(const RID &p_track_rid, const String &p_track_id, double p_length, const Vector3 &p_start_point, double p_track_width);
            void configure_path(
                    const RID &p_track_rid, const String &p_track_id, const PackedVector3Array &p_points,
                    double p_track_width, const RID &p_previous_track_rid = RID(), const RID &p_next_track_rid = RID());

            RID get_rid() const;
            String get_track_id() const;
            double get_length() const;
            Vector3 get_start_point() const;
            RID get_previous_track_rid() const;
            RID get_next_track_rid() const;
            Vector3 get_axis(double offset = 0.0) const;
            Vector3 get_world_position(double offset) const;
            bool sample_position(double offset, Vector3 &out_position) const;
            bool sample_axis(double offset, Vector3 &out_axis) const;
            bool sample_shape(double offset, Maszyna::TTrackShape &out_shape) const;
            double clamp_offset(double offset) const;

            const Maszyna::TTrackShape &get_shape() const;
            const Maszyna::TTrackParam &get_track_param() const;
    };

    class TrackManager : public Object {
            GDCLASS(TrackManager, Object)

        private:
            struct VehiclePlacement {
                RID track_rid;
                String track_id;
                double offset = 0.0;
            };

            mutable RID_Owner<Ref<VirtualTrack>> tracks;
            std::map<String, RID> named_tracks;
            std::map<TrainController *, VehiclePlacement> vehicle_placements;

            bool _update_track_name_mapping(const RID &track_rid, const String &previous_track_id, const String &track_id);
            void _report_duplicate_track_name(const String &track_id, const RID &existing_rid, const RID &requested_rid) const;
            TrainController *find_nearest_vehicle(
                    TrainController *vehicle, bool ahead, double *distance = nullptr, int *other_end = nullptr) const;
            bool resolve_track_position_internal(
                    const RID &track_rid, double offset, RID &resolved_track_rid, String &resolved_track_id,
                    double &resolved_offset, Vector3 *resolved_position = nullptr, Vector3 *resolved_axis = nullptr,
                    Maszyna::TTrackShape *resolved_shape = nullptr) const;

        protected:
            static void _bind_methods();

        public:
            TrackManager();
            ~TrackManager() override;

            static TrackManager *get_instance() {
                return Object::cast_to<TrackManager>(Engine::get_singleton()->get_singleton("TrackManager"));
            }

            RID add_track(double length, const Vector3 &start_point, const String &track_id = "");
            void modify_track(const RID &track_rid, double length, const Vector3 &start_point, const String &track_id = "");
            RID add_path_track(
                    const PackedVector3Array &points, const String &track_id = "", const RID &previous_track_rid = RID(),
                    const RID &next_track_rid = RID());
            void modify_path_track(
                    const RID &track_rid, const PackedVector3Array &points, const String &track_id = "",
                    const RID &previous_track_rid = RID(), const RID &next_track_rid = RID());
            void remove_track(const RID &track_rid);
            bool has_track(const String &track_id) const;
            Ref<VirtualTrack> get_track(const RID &track_rid) const;
            Ref<VirtualTrack> get_track_by_name(const String &track_id) const;
            Vector3 get_track_position(const RID &track_rid, double offset) const;
            Vector3 get_track_axis(const RID &track_rid, double offset) const;
            Dictionary resolve_track_position(const RID &track_rid, double offset) const;
            bool resolve_track_state(
                    const RID &track_rid, double offset, RID &resolved_track_rid, String &resolved_track_id,
                    double &resolved_offset, Vector3 *resolved_position = nullptr, Vector3 *resolved_axis = nullptr,
                    Maszyna::TTrackShape *resolved_shape = nullptr) const;

            void register_vehicle(TrainController *vehicle, const RID &track_rid, const String &track_id, double offset);
            void update_vehicle_offset(TrainController *vehicle, double offset);
            void remove_vehicle(TrainController *vehicle);

            Ref<VirtualTrack> get_vehicle_track(TrainController *vehicle) const;
            bool get_vehicle_offset(TrainController *vehicle, double &offset) const;
            TrainController *get_nearest_vehicle_ahead(
                    TrainController *vehicle, double *distance = nullptr, int *other_end = nullptr) const;
            TrainController *get_nearest_vehicle_behind(
                    TrainController *vehicle, double *distance = nullptr, int *other_end = nullptr) const;
    };
} // namespace godot
