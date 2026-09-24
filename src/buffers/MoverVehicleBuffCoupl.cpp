#include "MoverVehicleBuffCoupl.hpp"
#include "../mover/MoverBackend.hpp"
#include "VehicleBuffCoupl.hpp"

namespace godot {
    void MoverVehicleBuffCoupl::_bind_methods() {}

    /* The coupling flags of one end, read straight from the backend - this class is the only one
     * that may (Mover.h: TCoupling, coupling::). */
    bool MoverVehicleBuffCoupl::is_coupled(const End p_end) const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr && (mover->Couplers[p_end].CouplingFlag & coupling::coupler) != 0;
    }

    bool MoverVehicleBuffCoupl::is_brake_hose_connected(const End p_end) const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr && (mover->Couplers[p_end].CouplingFlag & coupling::brakehose) != 0;
    }

    bool MoverVehicleBuffCoupl::is_main_hose_connected(const End p_end) const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr && (mover->Couplers[p_end].CouplingFlag & coupling::mainhose) != 0;
    }

    bool MoverVehicleBuffCoupl::is_coupling_owner(const End p_end) const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr && mover->Couplers[p_end].Render;
    }

    VehicleBuffCoupl::End MoverVehicleBuffCoupl::get_connected_end(const End p_end) const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr && mover->Couplers[p_end].ConnectedNr == 1 ? END_REAR : END_FRONT;
    }


    void MoverVehicleBuffCoupl::_apply_configuration() {
        TMoverParameters *p_mover = get_mover();
        ASSERT_MOVER(p_mover);
        // LoadFIZ_BuffCoupl (Mover.cpp:10297): BuffCoupl2. -> rear coupler, BuffCoupl./BuffCoupl1. -> front
        TCoupling *coupler;
        if (get_buffer_location() == BufferLocation::BUFFER_LOCATION_BACK) {
            coupler = &p_mover->Couplers[1];
        } else {
            coupler = &p_mover->Couplers[0];
        }
        const double mass = train_controller_node->get_mass();
        const double max_velocity = train_controller_node->get_max_velocity();
        std::map<CouplerType, TCouplerType> coupler_types{
                {COUPLER_TYPE_AUTOMATIC, TCouplerType::Automatic},
                {COUPLER_TYPE_SCREW, TCouplerType::Screw},
                {COUPLER_TYPE_CHAIN, TCouplerType::Chain},
                {COUPLER_TYPE_BARE, TCouplerType::Bare},
                {COUPLER_TYPE_ARTICULATED, TCouplerType::Articulated},
        };

        const std::map<CouplerType, TCouplerType>::iterator lookup = coupler_types.find(get_coupler_type());
        const TCouplerType resolved_type = lookup != coupler_types.end() ? lookup->second : TCouplerType::NoCoupler;

        coupler->CouplerType = resolved_type;
        coupler->SpringKC = get_coupler_stiffness_k();
        coupler->DmaxC = get_coupler_max_compression_tolerance();
        coupler->FmaxC = get_coupler_max_tension_tolerance();
        coupler->SpringKB = get_buffer_stiffness_k();
        coupler->DmaxB = get_buffer_max_compression_tolerance();
        coupler->FmaxB = get_buffer_max_tension_tolerance();
        coupler->beta = get_damping_beta();
        coupler->AutomaticCouplingFlag = get_automatic_flag();
        coupler->AllowedFlag = get_allowed_flag();
        if (coupler->AllowedFlag < 0) {
            coupler->AllowedFlag = -coupler->AllowedFlag | coupling::permanent;
        }

        coupler->PowerCoupling = get_power_coupling();
        coupler->PowerFlag = get_power_flag();
        coupler->control_type = get_control_type().ascii();

        if (coupler->CouplerType != TCouplerType::NoCoupler && coupler->CouplerType != TCouplerType::Bare &&
            coupler->CouplerType != TCouplerType::Articulated) {

            coupler->SpringKC *= 1000;
            coupler->FmaxC *= 1000;
            coupler->SpringKB *= 1000;
            coupler->FmaxB *= 1000;
        } else if (coupler->CouplerType == TCouplerType::Bare) {
            coupler->SpringKC = (50.0 * mass) + (max_velocity / 0.05);
            coupler->DmaxC = 0.05;
            coupler->FmaxC = (100.0 * mass) + (2 * max_velocity);
            coupler->SpringKB = (60.0 * mass) + (max_velocity / 0.05);
            coupler->DmaxB = 0.05;
            coupler->FmaxB = (50.0 * mass) + (2.0 * max_velocity);
            coupler->beta = 0.3;
        } else if (coupler->CouplerType == TCouplerType::Articulated) {
            coupler->SpringKC = 4500 * 1000;
            coupler->DmaxC = 0.05;
            coupler->FmaxC = 850 * 1000;
            coupler->SpringKB = 9200 * 1000;
            coupler->DmaxB = 0.05;
            coupler->FmaxB = 320 * 1000;
            coupler->beta = 0.55;
        }

        if (get_buffer_location() == BufferLocation::BUFFER_LOCATION_BOTH) {
            // single entry for both couplers (Mover.cpp:10370); copy only the configuration, never the
            // runtime connection state (Connected, CouplingFlag, ...) of an already coupled vehicle
            TCoupling &rear = p_mover->Couplers[1];
            rear.CouplerType = coupler->CouplerType;
            rear.SpringKC = coupler->SpringKC;
            rear.DmaxC = coupler->DmaxC;
            rear.FmaxC = coupler->FmaxC;
            rear.SpringKB = coupler->SpringKB;
            rear.DmaxB = coupler->DmaxB;
            rear.FmaxB = coupler->FmaxB;
            rear.beta = coupler->beta;
            rear.AutomaticCouplingFlag = coupler->AutomaticCouplingFlag;
            rear.AllowedFlag = coupler->AllowedFlag;
            rear.PowerCoupling = coupler->PowerCoupling;
            rear.PowerFlag = coupler->PowerFlag;
            rear.control_type = coupler->control_type;
        }
    }


    void MoverVehicleBuffCoupl::_fill_config_dictionary(Dictionary &p_config) const {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        // Provide config dictionary entries
        p_config.set("get_buffer_stiffness_k()", get_buffer_stiffness_k());
        p_config.set("get_buffer_max_compression_tolerance()", get_buffer_max_compression_tolerance());
        p_config.set("get_buffer_max_tension_tolerance()", get_buffer_max_tension_tolerance());
        p_config.set("get_coupler_stiffness_k()", get_coupler_stiffness_k());
        p_config.set("get_coupler_max_compression_tolerance()", get_coupler_max_compression_tolerance());
        p_config.set("get_coupler_max_tension_tolerance()", get_coupler_max_tension_tolerance());
        p_config.set("get_damping_beta()", get_damping_beta());

        // New flags and control properties
        p_config.set("get_allowed_flag()", get_allowed_flag());
        p_config.set("get_automatic_flag()", get_automatic_flag());
        p_config.set("get_power_flag()", get_power_flag());
        p_config.set("get_power_coupling()", get_power_coupling());
        p_config.set("get_control_type()", get_control_type());
    }



    void MoverVehicleBuffCoupl::couple() {
        const TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);

        UtilityFunctions::push_warning(
                "[VehicleBuffCoupl] Coupling is not supported yet as it requires to handle logic between 2 vehicles "
                "simultaneously");
        log_warning(
                "[VehicleBuffCoupl] Coupling is not supported yet as it requires to handle logic between 2 vehicles "
                "simultaneously");
    }

    void MoverVehicleBuffCoupl::decouple() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);

        UtilityFunctions::push_warning(
                "[VehicleBuffCoupl] Decoupling is not supported yet as it requires to handle logic between 2 vehicles "
                "simultaneously");
        log_warning(
                "[VehicleBuffCoupl] Decoupling is not supported yet as it requires to handle logic between 2 vehicles "
                "simultaneously");
    }
} // namespace godot
