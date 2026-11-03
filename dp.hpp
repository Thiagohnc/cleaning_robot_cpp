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
std::vector<double> policy_evaluation(const TabularModel& model, const std::vector<int>& policy, double gamma);
void policy_improvement(const TabularModel& model, std::vector<int>& policy, const std::vector<double>& v, double gamma);

class ValueIteration : public TabularPolicy {
public:
    ValueIteration(const TabularModel& model, double gamma);
    void _learn();
};

class PolicyIteration : public TabularPolicy {
public:
    std::vector<int> policy;
    PolicyIteration(const TabularModel& model, double gamma);
    void _learn(const TabularModel& model, std::vector<int>& policy, double gamma);
};

} // namespace cleaning_robot
