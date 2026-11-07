#include "dp.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <chrono>
#include <limits>
#include <stdexcept>

using namespace std;

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

vector<double> policy_evaluation(const TabularModel& model, const vector<int>& policy, double gamma, double tol, bool silent) {
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    
    vector<double> v(model.n, 0);
    int iterations = 0;
    double max_diff = 2*tol;
    while(max_diff > tol) {
        max_diff = 0;
        for(int s = 0; s < model.n; s++) {
            int a = policy[s];
            double new_val = 0;
            for(int idx = model.P[a].indptr[s]; idx < model.P[a].indptr[s+1]; idx++) {
                int next_s = model.P[a].indices[idx];
                double p = model.P[a].data[idx];
                new_val += p * (model.R[next_s] + gamma * v[next_s]);
            }
            max_diff = max(fabs(v[s] - new_val), max_diff);
            v[s] = new_val;
        }
        iterations++;
    }

    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double duration = chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    if(not silent) cout << "Tempo: " << duration/1000 << "s | Iterações: " << iterations << endl;

    return v;
}

vector<double> policy_evaluation(const TabularModel& model, const vector<vector<double>>& policy, double gamma, double tol, bool silent) {
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    vector<double> v(model.n, 0);
    double max_diff = 2*tol;
    int iterations = 0;
    while(max_diff > tol) {
        max_diff = 0;
        for(int s = 0; s < model.n; s++) {
            double new_val = 0;
            for(int a = 0; a < (int)policy[s].size(); a++) {
                for(int idx = model.P[a].indptr[s]; idx < model.P[a].indptr[s+1]; idx++) {
                    int next_s = model.P[a].indices[idx];
                    double p = model.P[a].data[idx];
                    new_val += policy[s][a] * p * (model.R[next_s] + gamma * v[next_s]);
                }
            }
            max_diff = max(fabs(v[s] - new_val), max_diff);
            v[s] = new_val;
        }
        iterations++;
    }

    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double duration = chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    if(not silent) cout << "Tempo: " << duration/1000 << "s | Iterações: " << iterations << endl;

    return v;
}

// These exercises remain unfinished, matching the Python source.
ValueIteration::ValueIteration(const TabularModel& model, double gamma)
    : TabularPolicy(model, std::vector<int>(model.n)) {
    actions = _learn(model, gamma);
}

vector<int> ValueIteration::_learn(const TabularModel& model, double gamma, double tol) {
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    vector<double> v(model.n, 0);
    double max_diff = 2*tol;
    int iterations = 0;
    while(max_diff > tol) {
        max_diff = 0;
        for(int s = 0; s < model.n; s++) {
            double old_val = v[s];
            double best_val = numeric_limits<double>::lowest();
            for(int a = 0; a < 5; a++) {
                double val = 0;
                for(int idx = model.P[a].indptr[s]; idx < model.P[a].indptr[s+1]; idx++) {
                    int next_s = model.P[a].indices[idx];
                    double p = model.P[a].data[idx];
                    val += p * (model.R[next_s] + gamma * v[next_s]);
                }
                best_val = max(best_val, val);
            }
            v[s] = best_val;
            max_diff = max(fabs(v[s] - old_val), max_diff);
        }
        iterations++;
    }

    vector<int> policy(model.n, 0);
    for(int s = 0; s < model.n; s++) {
        int argmax = -1;
        double valmax = numeric_limits<double>::lowest();
        for(int a = 0; a < 5; a++) {
            double val = 0;
            for(int idx = model.P[a].indptr[s]; idx < model.P[a].indptr[s+1]; idx++) {
                int next_s = model.P[a].indices[idx];
                double p = model.P[a].data[idx];
                val += p * (model.R[next_s] + gamma * v[next_s]);
            }
            if(val > valmax) {
                valmax = val;
                argmax = a;
            }
        }
        policy[s] = argmax;
    }

    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double duration = chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    cout << "Tempo: " << duration/1000 << "s | Iterações: " << iterations << endl;

    return policy;
}

PolicyIteration::PolicyIteration(const TabularModel& model, double gamma)
    : TabularPolicy(model, vector<int>(model.n)) {
    actions = _learn(model, gamma);
}

vector<int> PolicyIteration::_learn(const TabularModel& model, double gamma) {
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    
    vector<int> policy(model.n, 0);
    int iterations = 0;
    bool stable = false;
    while (not stable) {
        stable = true;
        vector<double> v = policy_evaluation(model, policy, gamma, 1e-9, true);
        for(int s = 0; s < (int)policy.size(); s++) {
            int last_a = policy[s];
            double best_val = numeric_limits<double>::lowest();
            for(int a = 0; a < 5; a++) {
                double val = 0;
                for(int idx = model.P[a].indptr[s]; idx < model.P[a].indptr[s+1]; idx++) {
                    int next_s = model.P[a].indices[idx];
                    double p = model.P[a].data[idx];
                    val += p * (model.R[next_s] + gamma * v[next_s]);
                }
                if(val > best_val) {
                    best_val = val;
                    policy[s] = a;
                }
            }
            if(policy[s] != last_a) stable = false;
        }
        iterations++;
    }

    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double duration = chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    cout << "Tempo: " << duration/1000 << "s | Iterações: " << iterations << endl;

    return policy;
}

vector<double> simulate(const CleaningEnv& env, const vector<int> &policy, int n_steps, int rounds, int seed) {
    vector<double> rws;
    RNG rng(seed);

    for(int r = 0; r < rounds; r++) {
        double total_r = 0;
        State state = env.random_init(rng);
        for(int step = 0; step < n_steps; step++) {
            int s = state_index(env, state);
            int a = policy[s];
            state = env.next_state_sample(state, a, rng);
            total_r += env.reward(state);
        }
        rws.push_back(total_r / n_steps);
    }
    return rws;
}

} // namespace cleaning_robot
