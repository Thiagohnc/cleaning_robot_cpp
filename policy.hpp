#pragma once
#include "env.hpp"

namespace cleaning_robot {

class PolicyMixin {
public:
    virtual ~PolicyMixin() = default;
    virtual int act(const State& state, RNG& rng) const;
    int act(const State& state, int seed = -1) const;
};

class RandomPolicy : public PolicyMixin {
public:
    using PolicyMixin::act;
    int act(const State& state, RNG& rng) const override;
};

class GreedyPolicy : public PolicyMixin {
public:
    const CleaningEnv& env;
    GreedyPolicy(const CleaningEnv& env);
    using PolicyMixin::act;
    int act(const State& state, RNG& rng) const override;
};

} // namespace cleaning_robot
