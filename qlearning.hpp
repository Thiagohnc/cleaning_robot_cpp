#pragma once
#include "policy.hpp"
#include <vector>

namespace cleaning_robot {

// Model-free Q-learning exercise; no transition matrices are used.
class QLearning : public PolicyMixin {
public:
    const CleaningEnv& env;
    std::vector<std::vector<double>> Q;
    std::vector<int> actions;
    QLearning(const CleaningEnv& _env, double gamma, int n_steps, int seed = -1);
    using PolicyMixin::act;
    int act(const State& state, RNG& rng) const override;
    int eps_greedy_act(const State& state, double eps, RNG& rng) const;
    int best_action(const State& state) const;
    std::vector<int> _learn(const CleaningEnv& env, double gamma, int n_steps, int seed = -1);
};

} // namespace cleaning_robot
