#include "SceneryHUDMouseServer.hpp"
#include "legacy/e3d/E3DRenderingServer.hpp"
#include "rendering/MousePicking.hpp"
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    namespace {
        /// How far the outline reaches out of the model's silhouette, in metres. A lever stands
        /// metres to tens of metres away, where a cab control's few millimetres are not a pixel.
        constexpr double OUTLINE_WIDTH = 0.03;
        /// How strongly the hovered model itself is tinted with the outline colour - at a distance
        /// the ring alone is thin
        constexpr float OUTLINE_FILL_ALPHA = 0.15;
    } // namespace

    SceneryHUDMouseServer::SceneryHUDMouseServer() {
        outline_material = mouse_picking::outline_material(OUTLINE_WIDTH, OUTLINE_FILL_ALPHA);
    }

    void SceneryHUDMouseServer::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_camera", "camera_id"), &SceneryHUDMouseServer::set_camera);
        ClassDB::bind_method(D_METHOD("set_active", "active"), &SceneryHUDMouseServer::set_active);
        ClassDB::bind_method(
                D_METHOD("pickable_create", "instance", "pressed", "shift_pressed"),
                &SceneryHUDMouseServer::pickable_create);
        ClassDB::bind_method(D_METHOD("pickable_free", "pickable"), &SceneryHUDMouseServer::pickable_free);
        ClassDB::bind_method(D_METHOD("input", "event"), &SceneryHUDMouseServer::input);
        ClassDB::bind_method(D_METHOD("get_hovered_pickable"), &SceneryHUDMouseServer::get_hovered_pickable);
    }

    void SceneryHUDMouseServer::set_camera(const uint64_t p_camera_id) {
        camera = ObjectID(p_camera_id);
    }

    void SceneryHUDMouseServer::set_active(const bool p_active) {
        active = p_active;
        if (!active) {
            _set_hovered(RID());
        }
    }

    RID SceneryHUDMouseServer::pickable_create(
            const RID &p_instance, const Callable &p_pressed, const Callable &p_shift_pressed) {
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        pickables.insert(rid, Pickable{p_instance, p_pressed, p_shift_pressed});
        return rid;
    }

    void SceneryHUDMouseServer::pickable_free(const RID &p_pickable) {
        if (hovered == p_pickable) {
            _set_hovered(RID());
        }
        pickables.erase(p_pickable);
    }

    RID SceneryHUDMouseServer::get_hovered_pickable() const {
        return hovered;
    }

    bool SceneryHUDMouseServer::input(const Ref<InputEvent> &p_event) {
        if (!active) {
            return false;
        }
        const Ref<InputEventMouseMotion> motion = p_event;
        if (motion.is_valid()) {
            const Camera3D *view = Object::cast_to<Camera3D>(ObjectDB::get_instance(camera));
            const E3DRenderingServer *e3d = E3DRenderingServer::get_instance();
            if (view == nullptr || e3d == nullptr || !view->is_current() ||
                Input::get_singleton()->get_mouse_mode() != Input::MOUSE_MODE_VISIBLE ||
                view->get_viewport()->gui_get_hovered_control() != nullptr) {
                _set_hovered(RID());
                return false;
            }
            // the pickable whose model the ray through the cursor meets first
            const Vector3 from = view->project_ray_origin(motion->get_position());
            const Vector3 to = from + view->project_ray_normal(motion->get_position()) * view->get_far();
            RID nearest;
            double nearest_distance = view->get_far();
            for (const KeyValue<RID, Pickable> &entry: pickables) {
                const Dictionary hit = e3d->instance_intersect_segment(entry.value.instance, from, to);
                if (!hit.is_empty() && static_cast<double>(hit["distance"]) < nearest_distance) {
                    nearest_distance = hit["distance"];
                    nearest = entry.key;
                }
            }
            _set_hovered(nearest);
            return false;
        }

        const Ref<InputEventMouseButton> button = p_event;
        if (button.is_null() || button->get_button_index() != MOUSE_BUTTON_LEFT || !button->is_pressed() ||
            !hovered.is_valid()) {
            return false;
        }
        // every launcher of the model's name fires (basic_cell::on_click(), scene.cpp:36-43), the
        // second event with Shift (scene.cpp:691-704)
        const RID instance = pickables[hovered].instance;
        Vector<Callable> operations;
        for (const KeyValue<RID, Pickable> &entry: pickables) {
            if (entry.value.instance == instance) {
                operations.push_back(button->is_shift_pressed() ? entry.value.shift_pressed : entry.value.pressed);
            }
        }
        // collected first: an operation may free pickables
        for (const Callable &operation: operations) {
            if (operation.is_valid()) {
                operation.call();
            }
        }
        return true;
    }

    void SceneryHUDMouseServer::_set_hovered(const RID &p_pickable) {
        if (hovered == p_pickable) {
            return;
        }
        E3DRenderingServer *e3d = E3DRenderingServer::get_instance();
        ERR_FAIL_NULL(e3d);
        if (hovered.is_valid()) {
            e3d->instance_set_material_overlay(pickables[hovered].instance, Ref<Material>());
        }
        hovered = p_pickable;
        if (hovered.is_valid()) {
            e3d->instance_set_material_overlay(pickables[hovered].instance, outline_material);
        }
    }
} // namespace godot
