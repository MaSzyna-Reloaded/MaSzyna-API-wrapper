#include "../core/VehicleController.hpp"
#include "../mover/MoverBackend.hpp"
#include "../physics/RailVehicleServer.hpp"
#include "MoverVehicleRadio.hpp"

namespace godot {
    void MoverVehicleRadio::_bind_methods() {}

    bool MoverVehicleRadio::get_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Radio : false;
    }

    bool MoverVehicleRadio::get_powered() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Radio && (mover->Power24vIsAvailable || mover->Power110vIsAvailable) : false;
    }

    void MoverVehicleRadio::_do_process_component(const double p_delta) {
        if (const bool powered = get_powered(); powered != previous_powered) {
            previous_powered = powered;
            emit_signal(radio_toggled_signal, powered);
        }
    }

    void MoverVehicleRadio::radio(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->Radio = p_enabled;
    }

    // Original engine: TTrain::OnCommand_radiostopsend (Train.cpp:8149) - on the press, and only a
    // powered radio sends
    void MoverVehicleRadio::radio_stop(const bool p_pressed) {
        const VehicleController *controller = get_controller();
        RailVehicleServer *server = RailVehicleServer::get_instance();
        if (!p_pressed || !get_powered() || controller == nullptr || server == nullptr) {
            return;
        }
        server->vehicle_radio_stop(controller->get_rid());
    }

    // Original engine: TTrain::OnCommand_radiocall1send/3send (Train.cpp:8209-8236) - on the press,
    // from a powered radio on any channel but the one without calls
    void MoverVehicleRadio::radio_call(const bool p_pressed, const RadioCall p_call) {
        const VehicleController *controller = get_controller();
        RailVehicleServer *server = RailVehicleServer::get_instance();
        if (!p_pressed || !get_powered() || get_channel() == CHANNEL_NO_CALLS || controller == nullptr ||
            server == nullptr) {
            return;
        }
        server->vehicle_radio_call(controller->get_rid(), p_call);
    }

    // Original engine: TDynamicObject::RadioStop (DynObj.cpp:7229) - a vehicle with somebody
    // driving it, Radio-Stop fitted and the radio on brakes in emergency; the driver's
    // "Emergency_brake" command lands in RadiostopSwitch (Driver.cpp:4487, Mover.cpp:9462)
    void MoverVehicleRadio::radio_stop_receive() {
        TMoverParameters *mover = get_mover();
        const VehicleController *controller = get_controller();
        ASSERT_MOVER(mover);
        if (controller == nullptr || controller->get_driver_type() == VehicleController::DRIVER_NOBODY ||
            !mover->SecuritySystem.radiostop_available() || !mover->Radio) {
            return;
        }
        mover->RadiostopSwitch(true);
    }
} // namespace godot
