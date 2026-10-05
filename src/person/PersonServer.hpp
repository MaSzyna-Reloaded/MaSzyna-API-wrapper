#pragma once
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace godot {
    /* The people of the world - the player and every AI driver - by their handles. A person is
     * only a handle here: where it sits and in what role is VehicleServer's, what an AI driver
     * thinks is DriverSystem's, the player is PlayerServer's. */
    class PersonServer : public Object {
            GDCLASS(PersonServer, Object)

        public:
            static PersonServer *get_instance() {
                return Object::cast_to<PersonServer>(Engine::get_singleton()->get_singleton("PersonServer"));
            }

        private:
            HashSet<RID> persons;

        protected:
            static void _bind_methods();

        public:
            /* The person is gone (person: RID) - whoever keeps something of it lets it go */
            static const char *person_freed_signal;

            RID person_create();
            void person_free(const RID &p_person);
            bool person_exists(const RID &p_person) const;
    };
} // namespace godot
