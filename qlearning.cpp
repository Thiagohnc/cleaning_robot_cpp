#include "qlearning.hpp"
#include "model.hpp"
#include <stdexcept>
#include <vector>

using namespace std;

namespace cleaning_robot {

QLearning::QLearning(const CleaningEnv& _env, double gamma, int n_steps, int seed) : env(_env) {
    actions = _learn(env, gamma, n_steps, seed);

}

vector<int> QLearning::_learn(const CleaningEnv& env, double gamma, int n_steps, int seed) {
    const int total_states = n_states(env);
    const int total_actions = ACTIONS.size();
    const double alpha = 0.1;
    const double eps = 0.1;
    RNG rng(seed);

    Q.resize(total_states);
    for(int i = 0; i < total_states; i++) {
        Q[i].resize(total_actions, 0);
    }

    State state = env.random_init(seed);
    for(int episode = 1; episode <= n_steps; episode++) {
        int s = state_index(env, state);
        int a = eps_greedy_act(state, eps, rng);
        double r = env.reward(state);
        State next_state = env.next_state_sample(state, a, rng);
        int next_s = state_index(env, next_state);

        int next_a = best_action(next_state);

        Q[s][a] = Q[s][a] + alpha * (r + gamma * Q[next_s][next_a] - Q[s][a]);

        state = next_state;
    }

    vector<int> policy(total_states, 0);
    for(int s = 0; s < total_states; s++) {
        policy[s] = 0;
        for(int a = 1; a < total_actions; a++) {
            if(Q[s][a] > Q[s][policy[s]]) policy[s] = a; 
        }
    }

    return policy;
}

// Escolhe a ação utilizando uma política epsilon-greedy com epsilon = 0.1
int QLearning::eps_greedy_act(const State& state, double eps, RNG& rng) const {
    uniform_int_distribution<> rd_action(0,ACTIONS.size()-1);
    uniform_real_distribution<double> rd(0.0, 1.0);
    
    double r = rd(rng);

    if(r < eps) return rd_action(rng);
    return best_action(state);
}

int QLearning::best_action(const State& state) const {
    int _best_action = 0;
    int s = state_index(env, state);
    for(int a = 1; a < (int)ACTIONS.size(); a++) {
        if(Q[s][a] > Q[s][_best_action]) _best_action = a;
    }
    return _best_action;
}

int QLearning::act(const State& state, RNG& rng) const {
    return actions[state_index(env, state)];
}

} // namespace cleaning_robot
