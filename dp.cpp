#include "dp.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace cleaning_robot {

TabularPolicy::TabularPolicy(const TabularModel& model, const std::vector<int>& actions)
    : model(model), actions(actions) {}

int TabularPolicy::act(const State& state, RNG&) const {
    return actions[model.state_index(state)];
}

std::vector<int> tabulate(const TabularModel& model, const PolicyMixin& policy) {
    std::vector<int> actions(model.n);
    RNG rng = make_rng();
    for (int i = 0; i < model.n; ++i) actions[i] = policy.act(state_at(model.env, i), rng);
    return actions;
}

std::vector<double> policy_evaluation(const TabularModel& model, const std::vector<int>& policy, double gamma) {

}

void policy_improvement(const TabularModel& model, std::vector<int>& policy, const std::vector<double>& v, double gamma) {

}

// These exercises remain unfinished, matching the Python source.
ValueIteration::ValueIteration(const TabularModel& model, double)
    : TabularPolicy(model, std::vector<int>(model.n)) {
    throw std::logic_error("NotImplementedError: ValueIteration");
}

void ValueIteration::_learn() {
    throw std::logic_error("NotImplementedError: ValueIteration._learn");
}

PolicyIteration::PolicyIteration(const TabularModel& model, double)
    : TabularPolicy(model, std::vector<int>(model.n)), policy(model.n, N) {
    throw std::logic_error("NotImplementedError: PolicyIteration");
}

void PolicyIteration::_learn(const TabularModel& model, std::vector<int>& policy, double gamma) {
    while (true) {
        std::vector<double> v = policy_evaluation(model, policy, gamma);
        policy_improvement(model, policy, v, gamma);
    }
}

} // namespace cleaning_robot
