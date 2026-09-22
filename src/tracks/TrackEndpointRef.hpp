#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace godot {
    /* One endpoint of one registered track. Was TrackManager's own EndpointRef, an inner class of the
     * GDScript autoload - GDExtension has no equivalent, so it is a class of its own. */
    class TrackEndpointRef : public RefCounted {
            GDCLASS(TrackEndpointRef, RefCounted)

        private:
            RID track_rid;
            int endpoint_index = 0;

        protected:
            static void _bind_methods();

        public:
            void set_track_rid(const RID &p_track_rid);
            RID get_track_rid() const;
            void set_endpoint_index(int p_endpoint_index);
            int get_endpoint_index() const;
    };

    /* The tracks connected before and after one branch of a switch. Was TrackManager's own
     * BranchNeighbors. */
    class TrackBranchNeighbors : public RefCounted {
            GDCLASS(TrackBranchNeighbors, RefCounted)

        private:
            RID previous_track_rid;
            int previous_endpoint_index = 0;
            RID next_track_rid;
            int next_endpoint_index = 0;

        protected:
            static void _bind_methods();

        public:
            void set_previous_track_rid(const RID &p_track_rid);
            RID get_previous_track_rid() const;
            void set_previous_endpoint_index(int p_endpoint_index);
            int get_previous_endpoint_index() const;
            void set_next_track_rid(const RID &p_track_rid);
            RID get_next_track_rid() const;
            void set_next_endpoint_index(int p_endpoint_index);
            int get_next_endpoint_index() const;
    };
} // namespace godot
