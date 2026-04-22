#pragma once

#include "../maszyna/McZapkie/MOVER.h"
#include "RailVehicle.hpp"
#include "TrackManager.hpp"
#include "macros.hpp"
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {
    class TrainPart;

    class LegacyRailVehicle : public RailVehicle {
            GDCLASS(LegacyRailVehicle, RailVehicle)

        private:
            TMoverParameters *mover{};
            Dictionary state;
            Dictionary config;
            Dictionary internal_state;
            int debug_tick_counter = 0;
            double _external_move_accumulator = 0.0;
            double _movement_delta = 0.0;
            RID current_track_rid;
            String current_track_id;
            double current_track_offset = 0.0;

            double initial_velocity = 0.0;
            int cabin_number = 0;
            void initialize_mover();
            void _update_mover_config_if_dirty();
            void _handle_mover_update();
            void _sync_mover_neighbours();
            void sync_mover_coupling(RailVehicle *other_vehicle, Side self_side, Side other_side, bool attach);
            static int to_mover_end(Side side);

        protected:
            void _on_coupled(RailVehicle *other_vehicle, Side self_side, Side other_side) override;
            void _on_uncoupled(RailVehicle *other_vehicle, Side self_side, Side other_side) override;
            void _initialize() override;
            void _deinitialize() override;
            void _update(double delta) override;

            virtual void _do_update_internal_mover(TMoverParameters *mover) const;
            virtual void _do_fetch_config_from_mover(const TMoverParameters *mover, Dictionary &config) const;
            virtual void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state);
            virtual void _process_mover(double delta);
            virtual void _notification_after_mover_ready();
            virtual void _notification_before_mover_cleanup();
            virtual bool can_host_commands() const;
            virtual String get_command_target_id() const;

        public:
            ~LegacyRailVehicle() override;

            static const char *MOVER_CONFIG_CHANGED_SIGNAL;
            static const char *MOVER_INITIALIZED_SIGNAL;
            static const char *CONFIG_CHANGED;

            Dictionary get_config() const;
            void update_config(const Dictionary &p_config);
            Dictionary get_state();
            Dictionary &_get_state_internal();
            Dictionary get_mover_state();
            void update_state();
            void update_mover();
            TMoverParameters *get_mover() const;
            void set_mover_location(const Vector3 &position);
            Vector3 get_mover_location() const;
            void assign_track_rid(const RID &track_rid, const String &track_id = "");
            RID get_track_rid() const;
            void assign_track(const String &track_id);
            String get_track_id() const;
            void set_track_offset(double offset);
            double get_track_offset() const;

            static void _bind_methods();

            MAKE_MEMBER_GS(String, type_name, "");
            MAKE_MEMBER_GS_DIRTY(String, axle_arrangement, "");
            MAKE_MEMBER_GS_DIRTY(float, drag_coefficient, 0.0);
    };
} // namespace godot
