#include "TrackEndpointRef.hpp"

namespace godot {
    void TrackEndpointRef::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_track_rid", "track_rid"), &TrackEndpointRef::set_track_rid);
        ClassDB::bind_method(D_METHOD("get_track_rid"), &TrackEndpointRef::get_track_rid);
        ADD_PROPERTY(PropertyInfo(Variant::RID, "track_rid"), "set_track_rid", "get_track_rid");
        ClassDB::bind_method(D_METHOD("set_endpoint_index", "endpoint_index"), &TrackEndpointRef::set_endpoint_index);
        ClassDB::bind_method(D_METHOD("get_endpoint_index"), &TrackEndpointRef::get_endpoint_index);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "endpoint_index"), "set_endpoint_index", "get_endpoint_index");
    }

    void TrackEndpointRef::set_track_rid(const RID &p_track_rid) {
        track_rid = p_track_rid;
    }

    RID TrackEndpointRef::get_track_rid() const {
        return track_rid;
    }

    void TrackEndpointRef::set_endpoint_index(const int p_endpoint_index) {
        endpoint_index = p_endpoint_index;
    }

    int TrackEndpointRef::get_endpoint_index() const {
        return endpoint_index;
    }

    void TrackBranchNeighbors::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("set_previous_track_rid", "track_rid"), &TrackBranchNeighbors::set_previous_track_rid);
        ClassDB::bind_method(D_METHOD("get_previous_track_rid"), &TrackBranchNeighbors::get_previous_track_rid);
        ADD_PROPERTY(
                PropertyInfo(Variant::RID, "previous_track_rid"), "set_previous_track_rid", "get_previous_track_rid");
        ClassDB::bind_method(
                D_METHOD("set_previous_endpoint_index", "endpoint_index"),
                &TrackBranchNeighbors::set_previous_endpoint_index);
        ClassDB::bind_method(
                D_METHOD("get_previous_endpoint_index"), &TrackBranchNeighbors::get_previous_endpoint_index);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "previous_endpoint_index"), "set_previous_endpoint_index",
                "get_previous_endpoint_index");
        ClassDB::bind_method(D_METHOD("set_next_track_rid", "track_rid"), &TrackBranchNeighbors::set_next_track_rid);
        ClassDB::bind_method(D_METHOD("get_next_track_rid"), &TrackBranchNeighbors::get_next_track_rid);
        ADD_PROPERTY(PropertyInfo(Variant::RID, "next_track_rid"), "set_next_track_rid", "get_next_track_rid");
        ClassDB::bind_method(
                D_METHOD("set_next_endpoint_index", "endpoint_index"), &TrackBranchNeighbors::set_next_endpoint_index);
        ClassDB::bind_method(D_METHOD("get_next_endpoint_index"), &TrackBranchNeighbors::get_next_endpoint_index);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "next_endpoint_index"), "set_next_endpoint_index",
                "get_next_endpoint_index");
    }

    void TrackBranchNeighbors::set_previous_track_rid(const RID &p_track_rid) {
        previous_track_rid = p_track_rid;
    }

    RID TrackBranchNeighbors::get_previous_track_rid() const {
        return previous_track_rid;
    }

    void TrackBranchNeighbors::set_previous_endpoint_index(const int p_endpoint_index) {
        previous_endpoint_index = p_endpoint_index;
    }

    int TrackBranchNeighbors::get_previous_endpoint_index() const {
        return previous_endpoint_index;
    }

    void TrackBranchNeighbors::set_next_track_rid(const RID &p_track_rid) {
        next_track_rid = p_track_rid;
    }

    RID TrackBranchNeighbors::get_next_track_rid() const {
        return next_track_rid;
    }

    void TrackBranchNeighbors::set_next_endpoint_index(const int p_endpoint_index) {
        next_endpoint_index = p_endpoint_index;
    }

    int TrackBranchNeighbors::get_next_endpoint_index() const {
        return next_endpoint_index;
    }
} // namespace godot
