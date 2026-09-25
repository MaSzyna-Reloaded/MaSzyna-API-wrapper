#include "CabinHUDMouseSystem.hpp"
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    namespace {
        const Color OUTLINE_COLOR = Color(1.0, 0.85, 0.3);
        /// A control whose longest side is shorter than this, in metres, is a small one - a desk
        /// button or toggle. A thin ring is lost around it, so it gets a heavier one and its body
        /// is tinted too; a lever or a wheel is plain to see with a thin ring alone.
        constexpr double SMALL_CONTROL_SIZE = 0.06;
        /// How far the outline reaches out of the control's silhouette, in metres
        constexpr double SMALL_OUTLINE_WIDTH = 0.005;
        constexpr double LARGE_OUTLINE_WIDTH = 0.003;
        /// How strongly a small hovered control itself is tinted with the outline colour
        constexpr float SMALL_OUTLINE_FILL_ALPHA = 0.12;

        /* Godot's stencil outline (BaseMaterial3D::STENCIL_MODE_OUTLINE) is meant for the mesh's
         * own material: it writes the stencil and a grown next pass draws only outside it. The
         * cab's materials are shared between meshes, so the pair goes into the overlay instead -
         * the overlay writes the stencil and, with `p_fill_alpha`, tints the control. Both are
         * transparent, so the preset's render priorities (writer 0, outline 1) keep them in order,
         * and neither tests depth: the whole silhouette gets its ring, also where the control
         * sinks into its panel. */
        Ref<StandardMaterial3D> outline_material(const double p_width, const float p_fill_alpha) {
            Ref<StandardMaterial3D> material;
            material.instantiate();
            material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
            material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
            material->set_albedo(Color(OUTLINE_COLOR, p_fill_alpha));
            material->set_flag(BaseMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
            material->set_stencil_mode(BaseMaterial3D::STENCIL_MODE_OUTLINE);
            material->set_stencil_effect_color(OUTLINE_COLOR);
            material->set_stencil_effect_outline_thickness(p_width);
            const Ref<BaseMaterial3D> outline = material->get_next_pass();
            outline->set_flag(BaseMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
            return material;
        }
        /// How far from a small control the cursor may be and still take it - a toggle a few
        /// pixels wide is otherwise hard to hit
        constexpr double PICK_TOLERANCE_PIXELS = 12.0;
        /// How far from upright a control's axis of turning has to lie for the control to be
        /// turned by its top (sine of the axis' tilt, 0.87 - 60 degrees). A brake valve on a slanted
        /// panel leans less and keeps its handle.
        constexpr double LYING_AXIS_MIN_TILT = 0.87;
        /// Right and up, for a control whose movement cannot be seen on screen
        const Vector2 DEFAULT_DRAG_SIGNS = Vector2(1.0, -1.0);
        /// How far the mouse has to move after the button went down before the drag takes the axis
        /// it moved along most
        constexpr double DRAG_AXIS_LOCK_PIXELS = 6.0;

        MeshInstance3D *mesh_of(const ObjectID &p_mesh) {
            return Object::cast_to<MeshInstance3D>(ObjectDB::get_instance(p_mesh));
        }

        void collect_meshes(Node *p_node, Vector<ObjectID> &r_meshes) {
            for (int i = 0; i < p_node->get_child_count(true); i++) {
                Node *child = p_node->get_child(i, true);
                if (Object::cast_to<MeshInstance3D>(child) != nullptr) {
                    r_meshes.push_back(ObjectID(child->get_instance_id()));
                }
                collect_meshes(child, r_meshes);
            }
        }
    } // namespace

    const char *CabinHUDMouseSystem::control_hovered_signal = "control_hovered";
    const char *CabinHUDMouseSystem::control_unhovered_signal = "control_unhovered";
    const char *CabinHUDMouseSystem::control_state_changed_signal = "control_state_changed";

    CabinHUDMouseSystem::CabinHUDMouseSystem() {
        small_outline_material = outline_material(SMALL_OUTLINE_WIDTH, SMALL_OUTLINE_FILL_ALPHA);
        large_outline_material = outline_material(LARGE_OUTLINE_WIDTH, 0.0);
    }

    void CabinHUDMouseSystem::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_camera", "camera_id"), &CabinHUDMouseSystem::set_camera);
        ClassDB::bind_method(
                D_METHOD(
                        "control_create", "mesh_instance_id", "caption", "hints", "pressed", "released", "increase",
                        "decrease", "step_rotation", "step_offset", "drag", "drag_signs"),
                &CabinHUDMouseSystem::control_create);
        ClassDB::bind_method(D_METHOD("control_free", "control"), &CabinHUDMouseSystem::control_free);
        ClassDB::bind_method(
                D_METHOD("control_set_state", "control", "state"), &CabinHUDMouseSystem::control_set_state);
        ClassDB::bind_method(D_METHOD("occluder_create", "mesh_instance_id"), &CabinHUDMouseSystem::occluder_create);
        ClassDB::bind_method(D_METHOD("occluder_free", "occluder"), &CabinHUDMouseSystem::occluder_free);
        ClassDB::bind_method(D_METHOD("input", "event"), &CabinHUDMouseSystem::input);
        ClassDB::bind_method(D_METHOD("get_hovered_control"), &CabinHUDMouseSystem::get_hovered_control);

        BIND_CONSTANT(DRAG_STEP_PIXELS);

        ADD_SIGNAL(MethodInfo(
                control_hovered_signal, PropertyInfo(Variant::STRING, "caption"),
                PropertyInfo(Variant::STRING, "hints"), PropertyInfo(Variant::STRING, "state")));
        ADD_SIGNAL(MethodInfo(control_unhovered_signal));
        ADD_SIGNAL(MethodInfo(control_state_changed_signal, PropertyInfo(Variant::STRING, "state")));
    }

    void CabinHUDMouseSystem::set_camera(const uint64_t p_camera_id) {
        camera = ObjectID(p_camera_id);
    }

    RID CabinHUDMouseSystem::control_create(
            const uint64_t p_mesh_instance_id, const String &p_caption, const String &p_hints,
            const Callable &p_pressed, const Callable &p_released, const Callable &p_increase,
            const Callable &p_decrease, const Basis &p_step_rotation, const Vector3 &p_step_offset,
            const Callable &p_drag, const Vector2 &p_drag_signs) {
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        controls.insert(
                rid, Control{_pickable(p_mesh_instance_id, true), p_caption, p_hints, String(), p_pressed, p_released,
                             p_increase, p_decrease, p_drag, p_step_rotation, p_step_offset});
        const MeshInstance3D *mesh = mesh_of(ObjectID(p_mesh_instance_id));
        if (mesh != nullptr) {
            controls[rid].grip = _grip(controls[rid], mesh);
            controls[rid].drag_signs = p_drag_signs;
            controls[rid].small =
                    mesh->get_global_transform().xform(mesh->get_aabb()).get_longest_axis_size() < SMALL_CONTROL_SIZE;
        }
        return rid;
    }

    void CabinHUDMouseSystem::control_set_state(const RID &p_control, const String &p_state) {
        Control *control = controls.getptr(p_control);
        ERR_FAIL_NULL(control);
        if (control->state == p_state) {
            return;
        }
        control->state = p_state;
        if (hovered == p_control && !control->caption.is_empty()) {
            emit_signal(control_state_changed_signal, p_state);
        }
    }

    RID CabinHUDMouseSystem::occluder_create(const uint64_t p_mesh_instance_id) {
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        occluders.insert(rid, _pickable(p_mesh_instance_id, false));
        return rid;
    }

    void CabinHUDMouseSystem::occluder_free(const RID &p_occluder) {
        occluders.erase(p_occluder);
    }

    Vector3 CabinHUDMouseSystem::_grip(const Control &p_control, const MeshInstance3D *p_mesh) {
        const Quaternion step = Quaternion(p_control.step_rotation.orthonormalized());
        if (Math::is_zero_approx(step.get_angle())) {
            return Vector3();
        }
        const Vector3 axis = step.get_axis().normalized();
        const Transform3D to_control = p_mesh->get_global_transform().affine_inverse();
        Vector3 grip;
        double grip_distance = -1.0;
        for (const Part &part: p_control.pickable.parts) {
            const MeshInstance3D *part_mesh = mesh_of(part.mesh);
            if (part_mesh == nullptr) {
                continue;
            }
            const Transform3D transform = to_control * part_mesh->get_global_transform();
            for (const Vector3 &vertex: part.faces) {
                const Vector3 point = transform.xform(vertex);
                const double distance = (point - axis * axis.dot(point)).length();
                if (distance > grip_distance) {
                    grip_distance = distance;
                    grip = point;
                }
            }
        }
        return grip;
    }

    CabinHUDMouseSystem::Pickable
    CabinHUDMouseSystem::_pickable(const uint64_t p_mesh_instance_id, const bool p_with_children) {
        Vector<ObjectID> meshes;
        meshes.push_back(ObjectID(p_mesh_instance_id));
        MeshInstance3D *root = mesh_of(meshes[0]);
        if (p_with_children && root != nullptr) {
            collect_meshes(root, meshes);
        }
        Pickable pickable;
        for (const ObjectID &id: meshes) {
            const MeshInstance3D *mesh = mesh_of(id);
            pickable.parts.push_back(
                    Part{id, mesh != nullptr && mesh->get_mesh().is_valid() ? mesh->get_mesh()->get_faces()
                                                                            : PackedVector3Array()});
        }
        return pickable;
    }

    bool CabinHUDMouseSystem::_hit(
            const Pickable &p_pickable, const Vector3 &p_from, const Vector3 &p_to, double &r_distance,
            Vector3 &r_point) {
        bool hit = false;
        for (const Part &part: p_pickable.parts) {
            hit = _hit_part(part, p_from, p_to, r_distance, r_point) || hit;
        }
        return hit;
    }

    bool CabinHUDMouseSystem::_hit_part(
            const Part &p_pickable, const Vector3 &p_from, const Vector3 &p_to, double &r_distance, Vector3 &r_point) {
        const MeshInstance3D *mesh = mesh_of(p_pickable.mesh);
        if (mesh == nullptr || !mesh->is_visible_in_tree()) {
            return false;
        }
        const Transform3D transform = mesh->get_global_transform();
        const Transform3D inverse = transform.affine_inverse();
        const Vector3 from = inverse.xform(p_from);
        const Vector3 to = inverse.xform(p_to);
        // the box first: it rejects nearly every mesh, and one entered farther than the nearest
        // hit so far cannot hold a nearer triangle
        Vector3 entry;
        if (!mesh->get_aabb().intersects_segment(from, to, &entry) ||
            p_from.distance_to(transform.xform(entry)) >= r_distance) {
            return false;
        }

        // Moller-Trumbore over the segment from..to, t in [0, 1]
        const Vector3 direction = to - from;
        double nearest_t = 2.0;
        const Vector3 *faces = p_pickable.faces.ptr();
        for (int64_t i = 0; i + 2 < p_pickable.faces.size(); i += 3) {
            const Vector3 edge_1 = faces[i + 1] - faces[i];
            const Vector3 edge_2 = faces[i + 2] - faces[i];
            const Vector3 p = direction.cross(edge_2);
            const double determinant = edge_1.dot(p);
            if (Math::is_zero_approx(determinant)) {
                continue;
            }
            const Vector3 s = from - faces[i];
            const double u = s.dot(p) / determinant;
            if (u < 0.0 || u > 1.0) {
                continue;
            }
            const Vector3 q = s.cross(edge_1);
            const double v = direction.dot(q) / determinant;
            if (v < 0.0 || u + v > 1.0) {
                continue;
            }
            const double t = edge_2.dot(q) / determinant;
            if (t >= 0.0 && t < nearest_t) {
                nearest_t = t;
            }
        }
        if (nearest_t > 1.0) {
            return false;
        }
        const Vector3 point = transform.xform(from + direction * nearest_t);
        const double distance = p_from.distance_to(point);
        if (distance >= r_distance) {
            return false;
        }
        r_distance = distance;
        r_point = point;
        return true;
    }

    void CabinHUDMouseSystem::control_free(const RID &p_control) {
        if (held == p_control) {
            _end_hold();
        }
        if (hovered == p_control) {
            _set_hovered(RID());
        }
        controls.erase(p_control);
    }

    RID CabinHUDMouseSystem::get_hovered_control() const {
        return hovered;
    }

    bool CabinHUDMouseSystem::input(const Ref<InputEvent> &p_event) {
        const Ref<InputEventMouseMotion> motion = p_event;
        if (motion.is_valid()) {
            if (dragging) {
                /* The hand works a control either up-down or left-right, whichever its first
                 * movement goes - a brake valve's handle swings first down, then to the right -
                 * and keeps to it until it lets go. */
                Vector2 relative = motion->get_relative();
                if (drag_axis == DRAG_AXIS_NONE) {
                    drag_gesture += relative;
                    if (drag_gesture.length() < DRAG_AXIS_LOCK_PIXELS) {
                        return true;
                    }
                    drag_axis = Math::abs(drag_gesture.x) > Math::abs(drag_gesture.y) ? DRAG_AXIS_HORIZONTAL
                                                                                      : DRAG_AXIS_VERTICAL;
                    relative = drag_gesture;
                }
                const double travel =
                        drag_axis == DRAG_AXIS_HORIZONTAL ? relative.x * drag_signs.x : relative.y * drag_signs.y;
                if (controls[held].drag.is_valid()) {
                    controls[held].drag.call(travel);
                    return true;
                }
                /* Every position is a notch: one step at most per mouse motion, and what the hand
                 * pulled beyond it is dropped - the next position needs a pull of its own. Carrying
                 * the rest over let a quick move run from -1 through 0 to 1, the middle position
                 * impossible to stop at. */
                drag_travel += travel;
                if (drag_travel >= DRAG_STEP_PIXELS) {
                    drag_travel = 0.0;
                    controls[held].increase.call();
                } else if (drag_travel <= -DRAG_STEP_PIXELS) {
                    drag_travel = 0.0;
                    controls[held].decrease.call();
                }
                return true;
            }
            if (!held.is_valid()) {
                Vector3 point;
                const RID picked = _pick(motion->get_position(), point);
                _set_hovered(picked);
                hovered_point = point;
            }
            return false;
        }

        const Ref<InputEventMouseButton> button = p_event;
        if (button.is_null() || button->get_button_index() != MOUSE_BUTTON_LEFT) {
            return false;
        }
        if (button->is_pressed()) {
            if (!hovered.is_valid()) {
                return false;
            }
            held = hovered;
            if (controls[held].pressed.is_valid()) {
                controls[held].pressed.call();
            }
            // pressing may free the control, e.g. a cab change rebuilding the cabin
            if (held.is_valid() && (controls[held].increase.is_valid() || controls[held].drag.is_valid())) {
                // mouse_slider::bind() hides the cursor for the drag (drivermouseinput.cpp:116)
                dragging = true;
                drag_travel = 0.0;
                drag_axis = DRAG_AXIS_NONE;
                drag_gesture = Vector2();
                drag_signs = controls[held].drag_signs.is_zero_approx() ? _increase_signs(controls[held])
                                                                        : controls[held].drag_signs;
                drag_cursor_position = button->get_position();
                Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
            }
            return true;
        }
        if (!held.is_valid()) {
            return false;
        }
        if (controls[held].released.is_valid()) {
            controls[held].released.call();
        }
        // releasing may free the control as well
        if (held.is_valid()) {
            _end_hold();
        }
        return true;
    }

    void CabinHUDMouseSystem::_end_hold() {
        held = RID();
        if (!dragging) {
            return;
        }
        dragging = false;
        // mouse_slider::release() puts the cursor back where the drag began (drivermouseinput.cpp:135)
        Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);
        Input::get_singleton()->warp_mouse(drag_cursor_position);
    }

    Vector2 CabinHUDMouseSystem::_increase_signs(const Control &p_control) const {
        const Camera3D *view = Object::cast_to<Camera3D>(ObjectDB::get_instance(camera));
        const MeshInstance3D *mesh = mesh_of(p_control.pickable.parts[0].mesh);
        if (view == nullptr || mesh == nullptr) {
            return DEFAULT_DRAG_SIGNS;
        }
        /* Which point the drag follows - the same one wherever the cursor took the control, so a
         * control always drags the same way:
         * - a control that turns is held by its grip, the point of it farthest from the axis:
         *   SM42's brake valve (zasadniczy) is taken by its head or core right next to its axis,
         *   where the two sides move opposite ways, while the hand thinks of the handle
         *   (raczkaKranu) sticking out of it;
         * - one turning about a lying axis (a wheel, a lever on a horizontal pivot) by the top of
         *   its circle at the grip's distance instead - a wheel's farthest points run all round its
         *   rim, and the hand takes a wheel by its top. SM42's joint controller (jointctrl:
         *   nastawnik, a shaft with a hand wheel at each side) pushed forward below its hub turned
         *   backwards. Top means up in the world, not on screen;
         * - one that only slides moves the same everywhere, the grabbed point will do. */
        const Transform3D global = mesh->get_global_transform();
        Vector3 point = global.affine_inverse().xform(hovered_point);
        const Quaternion step = Quaternion(p_control.step_rotation.orthonormalized());
        if (!Math::is_zero_approx(step.get_angle())) {
            point = p_control.grip;
            const Vector3 axis = global.basis.xform(step.get_axis()).normalized();
            const Vector3 up = Vector3(0.0, 1.0, 0.0) - axis * axis.y;
            const Vector3 grip = global.xform(p_control.grip);
            const Vector3 hub = global.origin + axis * axis.dot(grip - global.origin);
            if (up.length() > LYING_AXIS_MIN_TILT) {
                point = global.affine_inverse().xform(hub + up.normalized() * hub.distance_to(grip));
            }
        }
        const Node3D *parent = mesh->get_parent_node_3d();
        const Transform3D parent_transform = parent == nullptr ? Transform3D() : parent->get_global_transform();
        const Transform3D transform = mesh->get_transform();
        const Vector3 before = parent_transform.xform(transform.xform(point));
        const Vector3 after =
                parent_transform.xform(transform.xform(p_control.step_rotation.xform(point)) + p_control.step_offset);
        const Vector2 direction = view->unproject_position(after) - view->unproject_position(before);
        // a control seen edge-on moves along the view ray and shows no direction on screen
        if (direction.is_zero_approx()) {
            return DEFAULT_DRAG_SIGNS;
        }
        // which way the grip goes on each mouse axis; the drag takes one of them (see input())
        return Vector2(SIGN(direction.x), SIGN(direction.y));
    }

    RID CabinHUDMouseSystem::_pick(const Vector2 &p_position, Vector3 &r_point) const {
        const Camera3D *view = Object::cast_to<Camera3D>(ObjectDB::get_instance(camera));
        if (view == nullptr || !view->is_current() ||
            Input::get_singleton()->get_mouse_mode() != Input::MOUSE_MODE_VISIBLE ||
            view->get_viewport()->gui_get_hovered_control() != nullptr) {
            return RID();
        }
        const RID exact = _pick_exact(view, p_position, r_point);
        if (exact.is_valid()) {
            return exact;
        }

        // the nearest control middle on screen, then a ray through it to be sure it shows there
        RID nearest;
        Vector2 nearest_middle;
        double nearest_distance = PICK_TOLERANCE_PIXELS;
        for (const KeyValue<RID, Control> &entry: controls) {
            const MeshInstance3D *mesh = mesh_of(entry.value.pickable.parts[0].mesh);
            if (mesh == nullptr || !mesh->is_visible_in_tree()) {
                continue;
            }
            const Vector3 middle = mesh->get_global_transform().xform(mesh->get_aabb().get_center());
            if (view->is_position_behind(middle)) {
                continue;
            }
            const Vector2 screen_middle = view->unproject_position(middle);
            const double distance = screen_middle.distance_to(p_position);
            if (distance < nearest_distance) {
                nearest_distance = distance;
                nearest = entry.key;
                nearest_middle = screen_middle;
            }
        }
        if (!nearest.is_valid() || _pick_exact(view, nearest_middle, r_point) != nearest) {
            return RID();
        }
        return nearest;
    }

    RID CabinHUDMouseSystem::_pick_exact(const Camera3D *p_view, const Vector2 &p_position, Vector3 &r_point) const {
        const Vector3 from = p_view->project_ray_origin(p_position);
        const Vector3 to = from + p_view->project_ray_normal(p_position) * p_view->get_far();

        RID nearest;
        double nearest_distance = p_view->get_far();
        for (const KeyValue<RID, Control> &entry: controls) {
            if (_hit(entry.value.pickable, from, to, nearest_distance, r_point)) {
                nearest = entry.key;
            }
        }
        // a control's own mesh registered as an occluder too is hit at the same distance, which
        // is not nearer - the control keeps it
        Vector3 occluded_point;
        for (const KeyValue<RID, Pickable> &entry: occluders) {
            if (_hit(entry.value, from, to, nearest_distance, occluded_point)) {
                return RID();
            }
        }
        return nearest;
    }

    void CabinHUDMouseSystem::_set_hovered(const RID &p_control) {
        if (hovered == p_control) {
            return;
        }
        if (hovered.is_valid()) {
            _set_outline(hovered, false);
            emit_signal(control_unhovered_signal);
        }
        hovered = p_control;
        if (!hovered.is_valid()) {
            return;
        }
        _set_outline(hovered, true);
        const Control &control = controls[hovered];
        if (!control.caption.is_empty()) {
            emit_signal(control_hovered_signal, control.caption, control.hints, control.state);
        }
    }

    void CabinHUDMouseSystem::_set_outline(const RID &p_control, const bool p_outlined) {
        const Control &control = controls[p_control];
        Ref<Material> material;
        if (p_outlined) {
            material = control.small ? small_outline_material : large_outline_material;
        }
        for (const Part &part: control.pickable.parts) {
            MeshInstance3D *mesh = mesh_of(part.mesh);
            if (mesh != nullptr) {
                mesh->set_material_overlay(material);
            }
        }
    }
} // namespace godot
