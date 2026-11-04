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

vector<double> policy_evaluation(const TabularModel& model, const vector<int>& policy, double gamma, double tol) {
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
    cout << "Tempo: " << duration/1000 << "s | Iterações: " << iterations << endl;

    return v;
}

vector<double> policy_evaluation(const TabularModel& model, const vector<vector<double>>& policy, double gamma, double tol) {
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    vector<double> v(model.n, 0);
    double max_diff = 1 - tol;
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
    cout << "Tempo: " << duration/1000 << "s | Iterações: " << iterations << endl;

    return v;
}

void policy_improvement(const TabularModel& model, std::vector<int>& policy, const std::vector<double>& v, double gamma) {
    return;
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
