#pragma once

#include "../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/rid_owner.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <map>

namespace godot {
    class TrainController;

    class VirtualTrack : public RefCounted {
            GDCLASS(VirtualTrack, RefCounted)

        private:
            RID track_rid;
            String track_id;
            double length = 0.0;
            Vector3 start_point = Vector3();
            Maszyna::TTrackShape shape;
            Maszyna::TTrackParam track_param;

        protected:
            static void _bind_methods();

        public:
            VirtualTrack() = default;

            void configure(const RID &p_track_rid, const String &p_track_id, double p_length, const Vector3 &p_start_point, double p_track_width);

            RID get_rid() const;
            String get_track_id() const;
            double get_length() const;
            Vector3 get_start_point() const;
            Vector3 get_axis() const;
            Vector3 get_world_position(double offset) const;

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

            void _report_duplicate_track_name(const String &track_id, const RID &existing_rid, const RID &requested_rid) const;
            TrainController *find_nearest_vehicle(
                    TrainController *vehicle, bool ahead, double *distance = nullptr, int *other_end = nullptr) const;

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
            void remove_track(const RID &track_rid);
            bool has_track(const String &track_id) const;
            Ref<VirtualTrack> get_track(const RID &track_rid) const;
            Ref<VirtualTrack> get_track_by_name(const String &track_id) const;
            Vector3 get_track_position(const RID &track_rid, double offset) const;

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
