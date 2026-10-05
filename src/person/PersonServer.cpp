#include "PersonServer.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    const char *PersonServer::person_freed_signal = "person_freed";

    void PersonServer::_bind_methods() {
        ClassDB::bind_method(D_METHOD("person_create"), &PersonServer::person_create);
        ClassDB::bind_method(D_METHOD("person_free", "person"), &PersonServer::person_free);
        ClassDB::bind_method(D_METHOD("person_exists", "person"), &PersonServer::person_exists);
        ADD_SIGNAL(MethodInfo(person_freed_signal, PropertyInfo(Variant::RID, "person")));
    }

    RID PersonServer::person_create() {
        const RID person = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        persons.insert(person);
        return person;
    }

    void PersonServer::person_free(const RID &p_person) {
        if (!persons.has(p_person)) {
            return;
        }
        persons.erase(p_person);
        emit_signal(person_freed_signal, p_person);
    }

    bool PersonServer::person_exists(const RID &p_person) const {
        return persons.has(p_person);
    }
} // namespace godot
