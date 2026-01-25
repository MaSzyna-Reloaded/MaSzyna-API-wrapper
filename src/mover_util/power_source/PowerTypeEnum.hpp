#pragma once

#include "godot_cpp/core/binder_common.hpp"

namespace godot {
    enum PowerType : int { NoPower, BioPower, MechPower, ElectricPower, SteamPower };
}

VARIANT_ENUM_CAST(PowerType)
