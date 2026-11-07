#pragma once
#include "model.hpp"
#include "policy.hpp"

namespace cleaning_robot {

class TabularPolicy : public PolicyMixin {
public:
    const TabularModel& model;
    std::vector<int> actions;

    TabularPolicy(const TabularModel& model, const std::vector<int>& actions);
    using PolicyMixin::act;
    int act(const State& state, RNG& rng) const override;
};

std::vector<int> tabulate(const TabularModel& model, const PolicyMixin& policy);
std::vector<double> policy_evaluation(const TabularModel& model, const std::vector<int>& policy, double gamma, double tol=1e-9, bool silent=false);
std::vector<double> policy_evaluation(const TabularModel& model, const std::vector<std::vector<double>>& policy, double gamma, double tol=1e-9, bool silent=false);
void policy_improvement(const TabularModel& model, std::vector<int>& policy, const std::vector<double>& v, double gamma);
std::vector<double> simulate(const CleaningEnv& env, const std::vector<int> &policy, int n_steps=1000000, int rounds=50, int seed=42);

class ValueIteration : public TabularPolicy {
public:
    ValueIteration(const TabularModel& model, double gamma);
    std::vector<int> _learn(const TabularModel& model, double gamma, double tol=1e-9);
};

class PolicyIteration : public TabularPolicy {
public:
    PolicyIteration(const TabularModel& model, double gamma);
    std::vector<int> _learn(const TabularModel& model, double gamma);
};

} // namespace cleaning_robot
