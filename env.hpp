#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace cleaning_robot {

const int N = 0, E = 1, S = 2, W = 3, STAY = 4;
const std::array<int, 5> ACTIONS = {N, E, S, W, STAY};
const std::array<std::string, 5> ACTION_NAMES = {"N", "E", "S", "W", "STAY"};
using Pos = std::pair<int, int>;
const std::array<Pos, 4> DIRS = {{{0, -1}, {1, 0}, {0, 1}, {-1, 0}}};
using RNG = std::mt19937_64;

struct State {
    Pos robot_pos;
    Pos dog_pos;
    int dog_face;
    bool dog_playing;
    std::optional<Pos> mess_pos;

    bool operator==(const State& other) const;
    bool operator<(const State& other) const;
};

struct Room {
    int width;
    int height;
    std::set<Pos> obstacles;

    static Room symmetric(int width, int height, int obs_w, int obs_h, int wall_gap, int stagger = 0);
    static Room centered(int width, int height, int obs_w, int obs_h);
    bool is_free(Pos pos) const;
};

inline const Room SMALL_ROOM = Room::centered(9, 5, 3, 1);
inline const Room README_ROOM = Room::symmetric(15, 8, 4, 2, 2);

// Cell indices are used internally; mess = -1 means the room is clean.
struct InternalState {
    int robot, dog, face;
    bool playing;
    int mess;
};

struct WanderState {
    int dog, face, mess;
    bool operator==(const WanderState& other) const;
};

struct WanderOutcome {
    WanderState state;
    double probability;
};

struct InternalOutcome {
    InternalState state;
    double probability;
};

struct PlayResult {
    int dog, face;
    bool playing;
};

struct TransitionResult {
    std::vector<InternalOutcome> distribution;
    std::string event; // Empty string is Python's None.
    std::optional<InternalState> start;
};

struct StepResult {
    State state;
    std::string event;
    std::vector<State> reboot;
};

RNG make_rng(int seed = -1);
double uniform(RNG& rng);
WanderState _sample(const std::vector<WanderOutcome>& outcomes, double u);
InternalState _sample(const std::vector<InternalOutcome>& outcomes, double u);

class CleaningEnv {
public:
    Room room;
    double p;
    int k;
    std::vector<Pos> cells;
    std::map<Pos, int> index;
    int n_cells;
    std::vector<std::array<int, 4>> move;
    std::vector<std::vector<bool>> line_of_sight;
    std::vector<std::vector<std::vector<bool>>> sees;
    std::vector<std::vector<std::int64_t>> king_dist, grid_dist;
    std::vector<std::vector<Pos>> chase;

    CleaningEnv(Room room = SMALL_ROOM, double p = 0.2, int k = 2);
    std::vector<int> _king_neighbors(int c) const;
    std::vector<std::vector<std::int64_t>> _bfs(bool king) const;
    int _face_towards(int a, int b) const;
    std::vector<WanderOutcome> _wander(int dog, int face, int mess) const;
    const std::vector<WanderOutcome>& _wander_k(int dog, int face, int mess, int k) const;
    PlayResult _play(int dog, int face, int robot) const;
    TransitionResult _transitions(InternalState s, int action) const;
    InternalState encode(const State& state) const;
    State decode(InternalState s) const;
    double reward(const State& state) const;
    std::map<State, double> next_state_probs(const State& state, int action) const;
    StepResult step(const State& state, int action, int seed = -1) const;
    StepResult step(const State& state, int action, RNG& rng) const;
    State next_state_sample(const State& state, int action, int seed = -1) const;
    State next_state_sample(const State& state, int action, RNG& rng) const;
    State random_init(int seed = -1) const;
    State random_init(RNG& rng) const;

private:
    mutable std::map<std::array<int, 4>, std::vector<WanderOutcome>> _wander_cache;
};

} // namespace cleaning_robot
