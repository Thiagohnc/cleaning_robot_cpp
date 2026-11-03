#include "policy.hpp"
#include <limits>
#include <stdexcept>

namespace cleaning_robot {

int PolicyMixin::act(const State&, RNG&) const {
    throw std::logic_error("NotImplementedError: PolicyMixin.act");
}

int PolicyMixin::act(const State& state, int seed) const {
    RNG rng = make_rng(seed);
    return act(state, rng);
}

int RandomPolicy::act(const State&, RNG& rng) const {
    return ACTIONS[std::uniform_int_distribution<int>(0, 4)(rng)];
}

GreedyPolicy::GreedyPolicy(const CleaningEnv& env) : env(env) {}

int GreedyPolicy::act(const State& state, RNG&) const {
    if (!state.mess_pos) return STAY;
    int robot = env.index.at(state.robot_pos);
    int mess = env.index.at(*state.mess_pos);
    std::int64_t best_distance = std::numeric_limits<std::int64_t>::max();
    int best_action = STAY;
    for (int a = 0; a < 4; ++a) {
        int next = env.move[robot][a];
        if (next >= 0) {
            std::int64_t distance = env.grid_dist[next][mess];
            if (best_action == STAY || distance < best_distance) {
                best_distance = distance;
                best_action = a;
            }
        }
    }
    return best_action;
}

} // namespace cleaning_robot
