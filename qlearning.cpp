#include "qlearning.hpp"
#include <stdexcept>

namespace cleaning_robot {

QLearning::QLearning(const CleaningEnv&, double, int, int) {
    throw std::logic_error("NotImplementedError: QLearning");
}

int QLearning::act(const State&, RNG&) const {
    throw std::logic_error("NotImplementedError: QLearning.act");
}

} // namespace cleaning_robot
