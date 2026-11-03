#pragma once
#include "env.hpp"

namespace cleaning_robot {

int n_states(const CleaningEnv& env);
std::vector<State> all_states(const CleaningEnv& env);
int state_index(const CleaningEnv& env, const State& state);
State state_at(const CleaningEnv& env, int i);

// Compressed sparse row storage, like scipy.sparse.csr_matrix.
// Row s contains entries indptr[s] through indptr[s + 1] - 1.
struct SparseMatrix {
    int n = 0;
    std::vector<int> indptr;
    std::vector<int> indices;
    std::vector<double> data;

    int nnz() const;
    double operator()(int row, int col) const;
};

std::vector<SparseMatrix> build_transition_matrices(const CleaningEnv& env);
std::vector<double> reward_vector(const CleaningEnv& env);

class TabularModel {
public:
    const CleaningEnv& env;
    int n;
    std::vector<SparseMatrix> P;
    std::vector<double> R;
    double build_time = 0;

    TabularModel(const CleaningEnv& env, bool verbose = true);
    TabularModel(const CleaningEnv& env, const std::vector<SparseMatrix>& P,
                 const std::vector<double>& R, bool verbose = true);
    int state_index(const State& state) const;
};

} // namespace cleaning_robot
