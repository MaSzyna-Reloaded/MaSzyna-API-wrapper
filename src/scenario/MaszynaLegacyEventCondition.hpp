#pragma once
#include "ScenarioEventCondition.hpp"
#include "ScenarioEventServer.hpp"
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {
    /// The original's event `condition` (event_conditions, Event.cpp:44-254) and a launcher's
    /// memcell condition (EvLaunch.cpp:100-130, 216-227): `probability` and `memcompare`. Every
    /// memory has to hold the masked fields; a text with a `*` is compared up to it
    /// (MemCell.cpp:153-164). `memcompareex`, `trackfree` and `trackoccupied` are not ported yet
    /// (TODO.md).
    class MaszynaLegacyEventCondition : public ScenarioEventCondition {
            GDCLASS(MaszynaLegacyEventCondition, ScenarioEventCondition)

        public:
            /// The part of a `memcompare` text that is compared is the part before it
            static constexpr const char *TEXT_WILDCARD = "*";

        private:
            double probability = 1.0;
            TypedArray<RID> memories;
            String text;
            double value1 = 0.0;
            double value2 = 0.0;
            BitField<ScenarioEventServer::MemoryField> mask = 0;

        protected:
            static void _bind_methods();

            bool test(const RID &p_event, const RID &p_activator) const override;

        public:
            /// Passes this often, 1 for always (Event.cpp:68-78)
            void set_probability(double p_probability);
            double get_probability() const;
            void set_memories(const TypedArray<RID> &p_memories);
            TypedArray<RID> get_memories() const;
            void set_text(const String &p_text);
            String get_text() const;
            void set_value1(double p_value1);
            double get_value1() const;
            void set_value2(double p_value2);
            double get_value2() const;
            void set_mask(BitField<ScenarioEventServer::MemoryField> p_mask);
            BitField<ScenarioEventServer::MemoryField> get_mask() const;
    };
} // namespace godot
