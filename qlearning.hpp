#pragma once
#include "policy.hpp"

namespace cleaning_robot {

// Model-free Q-learning exercise; no transition matrices are used.
class QLearning : public PolicyMixin {
public:
    QLearning(const CleaningEnv& env, double gamma, int n_steps, int seed = -1);
    using PolicyMixin::act;
    int act(const State& state, RNG& rng) const override;
};

} // namespace cleaning_robot
