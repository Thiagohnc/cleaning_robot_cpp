#include "qlearning.hpp"
#include "model.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;

namespace cleaning_robot {

QLearning::QLearning(const CleaningEnv& _env, double gamma, long long int n_steps, int seed) : env(_env) {
    actions = _learn(gamma, n_steps, seed);
}

vector<int> QLearning::_learn(double gamma, long long int n_steps, int seed) {
    const int total_states = n_states(env);
    const int total_actions = ACTIONS.size();
    const double alpha = 0.1;
    const double eps = 0.1;
    RNG rng(seed);

    Q.resize(total_states);
    for(int i = 0; i < total_states; i++) {
        if(gamma == 0.99) Q[i].resize(total_actions, -50);
        else if(gamma == 0.7) Q[i].resize(total_actions, -1.667);
        else {
            cout << "Algo deu errado" << endl;
            vector<int> nada(10);
            return nada;
        }
    }


    ofstream outfile;
    outfile.open("outputs/q3/qlearning");

    State state = env.random_init(seed);
    int total_r = 0;
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for(long long int step = 1; step <= n_steps; step++) {
        int s = state_index(env, state);
        int a = eps_greedy_act(state, eps, rng);
        double r = env.reward(state);
        State next_state = env.next_state_sample(state, a, rng);
        int next_s = state_index(env, next_state);

        int next_a = best_action(next_state);

        Q[s][a] = Q[s][a] + alpha * (r + gamma * Q[next_s][next_a] - Q[s][a]);

        state = next_state;

        total_r += r;
        if(step % 1000000000L == 0){
            cout << endl;
            outfile << step << "," << total_r << "," << mean_state_value() << endl;
            cout << step << "," << total_r << "," << mean_state_value() << endl;
            total_r = 0;
        }
    }

    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double duration = chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    cout << "Tempo: " << duration/1000 << "s | " << "Passos: " << n_steps << endl;

    vector<int> policy(total_states, 0);
    for(int s = 0; s < total_states; s++) {
        policy[s] = 0;
        for(int a = 1; a < total_actions; a++) {
            if(Q[s][a] > Q[s][policy[s]]) policy[s] = a; 
        }
    }
    
    outfile.close();

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
    return best_action(state_index(env, state));
}

int QLearning::best_action(int s) const {
    int _best_action = 0;
    for(int a = 1; a < (int)ACTIONS.size(); a++) {
        if(Q[s][a] > Q[s][_best_action]) _best_action = a;
    }
    return _best_action;
}

int QLearning::act(const State& state, RNG& rng) const {
    return actions[state_index(env, state)];
}

double QLearning::mean_state_value(void) const {
    double total_value = 0;
    const int total_states = n_states(env);

    for(int s = 0; s < total_states; s++) {
        int a = best_action(s);
        total_value += Q[s][a];
    }

    return total_value / total_states;
}

} // namespace cleaning_robot
