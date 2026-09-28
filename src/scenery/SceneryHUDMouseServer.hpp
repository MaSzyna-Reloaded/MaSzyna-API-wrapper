#pragma once
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace godot {
    /// Scenery models operated by mouse while walking: a left click on a model calls what its owner
    /// handed in, Shift+click the other one - the original's click on a scenery model in free-fly
    /// mode (drivermouseinput.cpp:331-353, basic_cell::on_click(), scene.cpp:33-43 and 691-704),
    /// which is how a hand-thrown switch is set. The model under the cursor is outlined as a cab
    /// control is (CabinHUDMouseSystem); the original shows nothing there.
    ///
    /// A pickable is an E3DRenderingServer instance and the operations its owner hands in; the
    /// server knows nothing of what they do. Picking runs on mouse motion only and casts the cursor
    /// ray against the registered instances alone - a handful per scenery, not its every model -
    /// of which only the ones built (streamed in) are tested.
    ///
    /// Unlike the original's pick buffer, nothing hides a model: one standing behind a building is
    /// picked through it.
    class SceneryHUDMouseServer : public Object {
            GDCLASS(SceneryHUDMouseServer, Object)

        public:
            static const char *pickable_hovered_signal;
            static const char *pickable_unhovered_signal;

            static SceneryHUDMouseServer *get_instance() {
                return Object::cast_to<SceneryHUDMouseServer>(
                        Engine::get_singleton()->get_singleton("SceneryHUDMouseServer"));
            }

        private:
            struct Pickable {
                    RID instance;
                    String caption;
                    String hints;
                    Callable pressed;
                    Callable shift_pressed;
            };

            HashMap<RID, Pickable> pickables;
            ObjectID camera;
            bool active = true;
            Ref<StandardMaterial3D> outline_material;
            RID hovered;

            void _set_hovered(const RID &p_pickable);

        protected:
            static void _bind_methods();

        public:
            SceneryHUDMouseServer();

            void set_camera(uint64_t p_camera_id);
            /// Inactive, nothing is picked and the outline is taken off - the original picks
            /// scenery only in free-fly mode, never from the cab
            void set_active(bool p_active);

            /// A click on the instance's model calls `p_pressed`, with Shift `p_shift_pressed`.
            /// Several pickables of one instance are all operated by a click on it. `p_caption` and
            /// `p_hints` (the keys that do the same) are what the tooltip shows while it is hovered.
            RID pickable_create(
                    const RID &p_instance, const String &p_caption, const String &p_hints, const Callable &p_pressed,
                    const Callable &p_shift_pressed);
            /// Frees the pickable; free it before its instance
            void pickable_free(const RID &p_pickable);

            /// Feeds one input event; true when the event operated a pickable and is consumed
            bool input(const Ref<InputEvent> &p_event);

            RID get_hovered_pickable() const;
    };
} // namespace godot
