#include "utils.hpp"
#include "../maszyna/utilities.h"

#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace libmaszyna::utils {
    template<typename EnumType>
    std::string enum_to_string() {
        std::vector<std::string> enum_names;
        enum_names.reserve(static_cast<int>(EnumType::COUNT)); // Pre allocating the required capacity before the loop
        for (int i = 0; i < static_cast<int>(EnumType::COUNT); ++i) {
            enum_names.push_back(std::to_string(i)); // Placeholder, replace it with actual enum name extraction
        }
        std::stringstream ss;
        for (size_t i = 0; i < enum_names.size(); ++i) {
            ss << enum_names[i];
            if (i < enum_names.size() - 1) {
                ss << ",";
            }
        }
        return ss.str();
    }

    // Backing engines for Maszyna::Random()/LocalRandom() (McZapkie's physics code calls these
    // unqualified, via the `using namespace Maszyna;` in hamulce.h). thread_local because
    // TrainPhysicsServer steps independent consists concurrently on WorkerThreadPool tasks — a
    // single shared engine would be a data race if any physics call chain reaches these (currently
    // none do on the live per-tick path; this is cheap insurance against a future change that
    // re-enables one that does).
    thread_local std::mt19937 random_engine;
    thread_local std::mt19937 local_random_engine;
} // namespace libmaszyna::utils

namespace Maszyna {
    double Random(double a, double b) {
        const unsigned long val = libmaszyna::utils::random_engine();
        return interpolate(a, b, static_cast<double>(val) / libmaszyna::utils::random_engine.max());
    }

    double LocalRandom(double a, double b) {
        const unsigned long val = libmaszyna::utils::local_random_engine();
        return interpolate(a, b, static_cast<double>(val) / libmaszyna::utils::local_random_engine.max());
    }
} // namespace Maszyna
