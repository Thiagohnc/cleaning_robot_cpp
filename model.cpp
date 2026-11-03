#include "model.hpp"
#include <chrono>
#include <iostream>

namespace cleaning_robot {

int n_states(const CleaningEnv& env) {
    int F = env.n_cells;
    return 2 * F * F * 4 * (F + 1);
}

std::vector<State> all_states(const CleaningEnv& env) {
    std::vector<State> states;
    states.reserve(n_states(env));
    for (int playing = 0; playing < 2; ++playing) {
        for (int robot = 0; robot < env.n_cells; ++robot) {
            for (int dog = 0; dog < env.n_cells; ++dog) {
                for (int face = 0; face < 4; ++face) {
                    for (int mess = -1; mess < env.n_cells; ++mess) {
                        states.push_back(env.decode({robot, dog, face, bool(playing), mess}));
                    }
                }
            }
        }
    }
    return states;
}

int state_index(const CleaningEnv& env, const State& state) {
    int F = env.n_cells;
    InternalState s = env.encode(state);
    return (((int(s.playing) * F + s.robot) * F + s.dog) * 4 + s.face) * (F + 1) + s.mess + 1;
}

// Inverse of state_index; lets large loops visit states without storing them all.
State state_at(const CleaningEnv& env, int i) {
    int F = env.n_cells;
    int mess = i % (F + 1) - 1;
    i /= F + 1;
    int face = i % 4;
    i /= 4;
    int dog = i % F;
    i /= F;
    int robot = i % F;
    i /= F;
    return env.decode({robot, dog, face, bool(i), mess});
}

int SparseMatrix::nnz() const {
    return int(data.size());
}

double SparseMatrix::operator()(int row, int col) const {
    for (int j = indptr[row]; j < indptr[row + 1]; ++j) {
        if (indices[j] == col) return data[j];
    }
    return 0.0;
}

std::vector<SparseMatrix> build_transition_matrices(const CleaningEnv& env) {
    int n = n_states(env);
    std::vector<SparseMatrix> P(5);
    for (int a : ACTIONS) {
        P[a].n = n;
        P[a].indptr.push_back(0);
    }
    for (int i = 0; i < n; ++i) {
        State state = state_at(env, i);
        for (int a : ACTIONS) {
            // Sorting by column matches scipy's CSR row ordering.
            std::map<int, double> row;
            for (const auto& entry : env.next_state_probs(state, a)) {
                row[state_index(env, entry.first)] += entry.second;
            }
            for (const auto& entry : row) {
                P[a].indices.push_back(entry.first);
                P[a].data.push_back(entry.second);
            }
            P[a].indptr.push_back(P[a].nnz());
        }
    }
    return P;
}

std::vector<double> reward_vector(const CleaningEnv& env) {
    std::vector<double> R(n_states(env));
    for (int i = 0; i < int(R.size()); ++i) R[i] = env.reward(state_at(env, i));
    return R;
}

TabularModel::TabularModel(const CleaningEnv& env, bool verbose) : env(env), n(n_states(env)) {
    auto start = std::chrono::steady_clock::now();
    P = build_transition_matrices(env);
    R = reward_vector(env);
    build_time = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    if (verbose) {
        int transitions = 0;
        for (const auto& matrix : P) transitions += matrix.nnz();
        std::cout << "model: " << n << " states, " << transitions
                  << " transitions, built in " << build_time << "s\n";
    }
}

TabularModel::TabularModel(const CleaningEnv& env, const std::vector<SparseMatrix>& P,
                         const std::vector<double>& R, bool)
    : env(env), n(n_states(env)), P(P), R(R) {}

int TabularModel::state_index(const State& state) const {
    return cleaning_robot::state_index(env, state);
}

} // namespace cleaning_robot
