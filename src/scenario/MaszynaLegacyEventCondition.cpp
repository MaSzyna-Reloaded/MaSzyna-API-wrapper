#include "MaszynaLegacyEventCondition.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void MaszynaLegacyEventCondition::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_probability", "probability"), &MaszynaLegacyEventCondition::set_probability);
        ClassDB::bind_method(D_METHOD("get_probability"), &MaszynaLegacyEventCondition::get_probability);
        ClassDB::bind_method(D_METHOD("set_memories", "memories"), &MaszynaLegacyEventCondition::set_memories);
        ClassDB::bind_method(D_METHOD("get_memories"), &MaszynaLegacyEventCondition::get_memories);
        ClassDB::bind_method(D_METHOD("set_text", "text"), &MaszynaLegacyEventCondition::set_text);
        ClassDB::bind_method(D_METHOD("get_text"), &MaszynaLegacyEventCondition::get_text);
        ClassDB::bind_method(D_METHOD("set_value1", "value1"), &MaszynaLegacyEventCondition::set_value1);
        ClassDB::bind_method(D_METHOD("get_value1"), &MaszynaLegacyEventCondition::get_value1);
        ClassDB::bind_method(D_METHOD("set_value2", "value2"), &MaszynaLegacyEventCondition::set_value2);
        ClassDB::bind_method(D_METHOD("get_value2"), &MaszynaLegacyEventCondition::get_value2);
        ClassDB::bind_method(D_METHOD("set_mask", "mask"), &MaszynaLegacyEventCondition::set_mask);
        ClassDB::bind_method(D_METHOD("get_mask"), &MaszynaLegacyEventCondition::get_mask);

        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "probability", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_probability",
                "get_probability");
        ADD_PROPERTY(
                PropertyInfo(Variant::ARRAY, "memories", PROPERTY_HINT_ARRAY_TYPE, "RID"), "set_memories",
                "get_memories");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "value1"), "set_value1", "get_value1");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "value2"), "set_value2", "get_value2");
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "mask", PROPERTY_HINT_FLAGS, "Text,Value 1,Value 2"), "set_mask",
                "get_mask");
    }

    /// event_conditions::test() (Event.cpp:62-176): the probability first, then the memories
    bool MaszynaLegacyEventCondition::test(const RID &p_event, const RID &p_activator) const {
        if (UtilityFunctions::randf() > probability) {
            return false;
        }
        const ScenarioEventServer *server = ScenarioEventServer::get_instance();
        ERR_FAIL_NULL_V(server, false);
        const int wildcard = text.find(TEXT_WILDCARD);
        for (int i = 0; i < memories.size(); i++) {
            const RID memory = memories[i];
            if (mask.has_flag(ScenarioEventServer::MEMORY_FIELD_TEXT)) {
                const String memory_text = server->memory_get_text(memory);
                const bool equal = wildcard < 0 ? memory_text == text
                                                : memory_text.substr(0, wildcard) == text.substr(0, wildcard);
                if (!equal) {
                    return false;
                }
            }
            if (mask.has_flag(ScenarioEventServer::MEMORY_FIELD_VALUE1) &&
                !(server->memory_get_value1(memory) == value1)) {
                return false;
            }
            if (mask.has_flag(ScenarioEventServer::MEMORY_FIELD_VALUE2) &&
                !(server->memory_get_value2(memory) == value2)) {
                return false;
            }
        }
        return true;
    }

    void MaszynaLegacyEventCondition::set_probability(const double p_probability) {
        probability = p_probability;
    }

    double MaszynaLegacyEventCondition::get_probability() const {
        return probability;
    }

    void MaszynaLegacyEventCondition::set_memories(const TypedArray<RID> &p_memories) {
        memories = p_memories;
    }

    TypedArray<RID> MaszynaLegacyEventCondition::get_memories() const {
        return memories;
    }

    void MaszynaLegacyEventCondition::set_text(const String &p_text) {
        text = p_text;
    }

    String MaszynaLegacyEventCondition::get_text() const {
        return text;
    }

    void MaszynaLegacyEventCondition::set_value1(const double p_value1) {
        value1 = p_value1;
    }

    double MaszynaLegacyEventCondition::get_value1() const {
        return value1;
    }

    void MaszynaLegacyEventCondition::set_value2(const double p_value2) {
        value2 = p_value2;
    }

    double MaszynaLegacyEventCondition::get_value2() const {
        return value2;
    }

    void MaszynaLegacyEventCondition::set_mask(const BitField<ScenarioEventServer::MemoryField> p_mask) {
        mask = p_mask;
    }

    BitField<ScenarioEventServer::MemoryField> MaszynaLegacyEventCondition::get_mask() const {
        return mask;
    }
} // namespace godot
