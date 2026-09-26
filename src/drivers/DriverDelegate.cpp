#include "DriverDelegate.hpp"

namespace godot {
    void DriverDelegate::_bind_methods() {
        GDVIRTUAL_BIND(_driver_attached, "driver");
        GDVIRTUAL_BIND(_driver_detached, "driver");
        GDVIRTUAL_BIND(_handle_command, "driver", "command", "value1", "value2", "position");
        GDVIRTUAL_BIND(_update, "driver");
    }

    void DriverDelegate::driver_attached(const RID &p_driver) {
        GDVIRTUAL_CALL(_driver_attached, p_driver);
    }

    void DriverDelegate::driver_detached(const RID &p_driver) {
        GDVIRTUAL_CALL(_driver_detached, p_driver);
    }

    void DriverDelegate::handle_command(
            const RID &p_driver, const String &p_command, const double p_value1, const double p_value2,
            const Vector3 &p_position) {
        GDVIRTUAL_CALL(_handle_command, p_driver, p_command, p_value1, p_value2, p_position);
    }

    void DriverDelegate::update(const RID &p_driver) {
        GDVIRTUAL_CALL(_update, p_driver);
    }
} // namespace godot
