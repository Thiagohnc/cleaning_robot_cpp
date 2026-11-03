#include "env.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace cleaning_robot {

// Python's integer division rounds down.
int half_floor(int n) { return int(std::floor(n / 2.0)); }

bool State::operator==(const State& other) const {
    return robot_pos == other.robot_pos && dog_pos == other.dog_pos &&
           dog_face == other.dog_face && dog_playing == other.dog_playing && mess_pos == other.mess_pos;
}

// Allows State to be a dictionary key, like the frozen Python dataclass.
bool State::operator<(const State& other) const {
    if (robot_pos != other.robot_pos) return robot_pos < other.robot_pos;
    if (dog_pos != other.dog_pos) return dog_pos < other.dog_pos;
    if (dog_face != other.dog_face) return dog_face < other.dog_face;
    if (dog_playing != other.dog_playing) return dog_playing < other.dog_playing;
    return mess_pos < other.mess_pos;
}

bool WanderState::operator==(const WanderState& other) const {
    return dog == other.dog && face == other.face && mess == other.mess;
}

Room Room::symmetric(int width, int height, int obs_w, int obs_h, int wall_gap, int stagger) {
    Room room{width, height, {}};
    const int y0 = half_floor(height - obs_h);
    for (auto [x, y] : {Pos{wall_gap, y0 - stagger}, Pos{width - wall_gap - obs_w, y0 + stagger}})
        for (int dx = 0; dx < obs_w; ++dx)
            for (int dy = 0; dy < obs_h; ++dy) room.obstacles.emplace(x + dx, y + dy);
    return room;
}
Room Room::centered(int width, int height, int obs_w, int obs_h) {
    Room room{width, height, {}};
    const int x0 = half_floor(width - obs_w), y0 = half_floor(height - obs_h);
    for (int dx = 0; dx < obs_w; ++dx)
        for (int dy = 0; dy < obs_h; ++dy) room.obstacles.emplace(x0 + dx, y0 + dy);
    return room;
}
bool Room::is_free(Pos pos) const {
    const auto [x, y] = pos;
    return x >= 0 && x < width && y >= 0 && y < height && !obstacles.count(pos);
}
RNG make_rng(int seed) {
    if (seed >= 0) return RNG(seed);
    return RNG(std::random_device{}());
}
double uniform(RNG& rng) {
    return std::uniform_real_distribution<double>(0.0, 1.0)(rng);
}

CleaningEnv::CleaningEnv(Room room_, double p_, int k_) : room(std::move(room_)), p(p_), k(k_) {
    for (int y = 0; y < room.height; ++y)
        for (int x = 0; x < room.width; ++x)
            if (room.is_free({x, y})) {
                index[{x, y}] = static_cast<int>(cells.size());
                cells.emplace_back(x, y);
            }
    n_cells = static_cast<int>(cells.size());
    move.resize(n_cells);
    for (int i = 0; i < n_cells; ++i) {
        const auto [x, y] = cells[i];
        for (int d = 0; d < 4; ++d) {
            const auto [dx, dy] = DIRS[d];
            auto it = index.find({x + dx, y + dy});
            move[i][d] = it == index.end() ? -1 : it->second;
        }
    }
    line_of_sight.assign(n_cells, std::vector<bool>(n_cells, true));
    constexpr double eps = 1e-9;
    for (int a = 0; a < n_cells; ++a) {
        const auto [x0, y0] = cells[a];
        for (int b = a + 1; b < n_cells; ++b) {
            const auto [x1, y1] = cells[b];
            const int n = 8 * std::max(std::abs(x1 - x0), std::abs(y1 - y0)) + 1;
            for (int i = 0; i < n; ++i) {
                const double t = static_cast<double>(i) / (n - 1);
                const double px = x0 + t * (x1 - x0) + 0.5, py = y0 + t * (y1 - y0) + 0.5;
                bool blocked = false;
                for (int cx : {int(std::floor(px - eps)), int(std::floor(px + eps))})
                    for (int cy : {int(std::floor(py - eps)), int(std::floor(py + eps))})
                        blocked = blocked || room.obstacles.count({cx, cy});
                if (blocked) { line_of_sight[a][b] = line_of_sight[b][a] = false; break; }
            }
        }
    }
    sees.assign(n_cells, std::vector<std::vector<bool>>(4));
    for (int a = 0; a < n_cells; ++a) {
        const auto [x0, y0] = cells[a];
        for (int f = 0; f < 4; ++f) {
            sees[a][f].resize(n_cells);
            const auto [fx, fy] = DIRS[f];
            for (int b = 0; b < n_cells; ++b) {
                const int dx = cells[b].first - x0, dy = cells[b].second - y0;
                const int forward = dx * fx + dy * fy, lateral = std::abs(dx * fy - dy * fx);
                sees[a][f][b] = a == b || (forward > 0 && lateral <= forward && line_of_sight[a][b]);
            }
        }
    }
    king_dist = _bfs(true);
    grid_dist = _bfs(false);
    chase.assign(n_cells, std::vector<std::pair<int, int>>(n_cells));
    for (int dog = 0; dog < n_cells; ++dog)
        for (int target = 0; target < n_cells; ++target) {
            int c = dog;
            for (int step = 0; step < 2 && c != target; ++step) {
                int best = c;
                for (int n : _king_neighbors(c)) {
                    int dx = cells[n].first - cells[target].first;
                    int dy = cells[n].second - cells[target].second;
                    int bx = cells[best].first - cells[target].first;
                    int by = cells[best].second - cells[target].second;
                    if (king_dist[n][target] < king_dist[best][target] ||
                        (king_dist[n][target] == king_dist[best][target] && dx * dx + dy * dy < bx * bx + by * by)) {
                        best = n;
                    }
                }
                c = best;
            }
            chase[dog][target] = {c, _face_towards(c, target)};
        }
}

std::vector<int> CleaningEnv::_king_neighbors(int c) const {
    const auto [x, y] = cells[c];
    std::vector<int> out;
    for (int dx : {-1, 0, 1}) for (int dy : {-1, 0, 1}) {
        auto it = index.find({x + dx, y + dy});
        if ((dx == 0 && dy == 0) || it == index.end()) continue;
        if (dx && dy && (!index.count({x + dx, y}) || !index.count({x, y + dy}))) continue;
        out.push_back(it->second);
    }
    return out;
}
std::vector<std::vector<std::int64_t>> CleaningEnv::_bfs(bool king) const {
    std::vector<std::vector<std::int64_t>> dist(n_cells, std::vector<std::int64_t>(n_cells, std::numeric_limits<std::int64_t>::max()));
    for (int src = 0; src < n_cells; ++src) {
        dist[src][src] = 0;
        std::queue<int> queue;
        queue.push(src);
        while (!queue.empty()) {
            int c = queue.front(); queue.pop();
            std::vector<int> neighbors;
            if (king) neighbors = _king_neighbors(c);
            else for (int n : move[c]) if (n >= 0) neighbors.push_back(n);
            for (int n : neighbors) if (dist[src][n] > dist[src][c] + 1) {
                dist[src][n] = dist[src][c] + 1;
                queue.push(n);
            }
        }
    }
    return dist;
}
int CleaningEnv::_face_towards(int a, int b) const {
    const int dx = cells[b].first - cells[a].first, dy = cells[b].second - cells[a].second;
    if (!dx && !dy) return -1;
    if (std::abs(dx) >= std::abs(dy)) return dx > 0 ? E : W;
    return dy > 0 ? S : N;
}

WanderState _sample(const std::vector<WanderOutcome>& outcomes, double u) {
    for (const auto& outcome : outcomes) {
        u -= outcome.probability;
        if (u < 0) return outcome.state;
    }
    return outcomes.back().state;
}

InternalState _sample(const std::vector<InternalOutcome>& outcomes, double u) {
    for (const auto& outcome : outcomes) {
        u -= outcome.probability;
        if (u < 0) return outcome.state;
    }
    return outcomes.back().state;
}

std::vector<WanderOutcome> CleaningEnv::_wander(int dog, int face, int mess) const {
    std::vector<WanderOutcome> out;
    if (p < 1) {
        int next = move[dog][face];
        if (next < 0) out.push_back({{dog, (face + 2) % 4, mess}, 1 - p});
        else out.push_back({{next, face, mess}, 1 - p});
    }
    if (p > 0) {
        int new_mess = mess < 0 ? dog : mess;
        for (int f = 0; f < 4; ++f) {
            if (f != face) out.push_back({{dog, f, new_mess}, p / 3});
        }
    }
    return out;
}

const std::vector<WanderOutcome>& CleaningEnv::_wander_k(int dog, int face, int mess, int rounds) const {
    std::array<int, 4> key = {dog, face, mess, rounds};
    if (_wander_cache.count(key)) return _wander_cache[key];

    std::vector<WanderOutcome> dist;
    if (rounds == 0) {
        dist.push_back({{dog, face, mess}, 1.0});
    } else {
        for (const auto& first : _wander(dog, face, mess)) {
            WanderState s = first.state;
            for (const auto& next : _wander_k(s.dog, s.face, s.mess, rounds - 1)) {
                // Merge repeated outcomes, like dict.get(next, 0) in Python.
                bool found = false;
                for (auto& entry : dist) {
                    if (entry.state == next.state) {
                        entry.probability += first.probability * next.probability;
                        found = true;
                        break;
                    }
                }
                if (!found) dist.push_back({next.state, first.probability * next.probability});
            }
        }
    }
    _wander_cache[key] = dist;
    return _wander_cache[key];
}

PlayResult CleaningEnv::_play(int dog, int face, int robot) const {
    if (line_of_sight[dog][robot]) {
        auto [c, f] = chase[dog][robot];
        return {c, f < 0 ? face : f, true};
    }
    int c = dog;
    for (int i = 0; i < 2; ++i) {
        if (move[c][face] < 0) break;
        c = move[c][face];
    }
    if (move[c][face] < 0 && !line_of_sight[c][robot]) return {c, face, false};
    return {c, face, true};
}

TransitionResult CleaningEnv::_transitions(InternalState s, int action) const {
    int next = action != STAY ? move[s.robot][action] : -1;
    bool moved = next >= 0;
    if (moved) s.robot = next;
    if (s.mess == s.robot) s.mess = -1;

    std::string event;
    if (!s.playing && moved && sees[s.dog][s.face][s.robot]) {
        s.playing = true;
        event = "spotted";
    }
    if (s.playing) {
        PlayResult play = _play(s.dog, s.face, s.robot);
        s.dog = play.dog;
        s.face = play.face;
        s.playing = play.playing;
        if (s.dog == s.robot) {
            s.playing = false;
            std::vector<InternalOutcome> out;
            for (const auto& outcome : _wander_k(s.dog, s.face, s.mess, k)) {
                WanderState w = outcome.state;
                out.push_back({{s.robot, w.dog, w.face, false, w.mess}, outcome.probability});
            }
            return {out, "captured", s};
        }
        if (!s.playing) event = "lost";
        return {{{s, 1.0}}, event, std::nullopt};
    }

    std::vector<InternalOutcome> out;
    for (const auto& outcome : _wander(s.dog, s.face, s.mess)) {
        WanderState w = outcome.state;
        out.push_back({{s.robot, w.dog, w.face, false, w.mess}, outcome.probability});
    }
    return {out, event, std::nullopt};
}

InternalState CleaningEnv::encode(const State& state) const {
    int mess = state.mess_pos ? index.at(*state.mess_pos) : -1;
    return {index.at(state.robot_pos), index.at(state.dog_pos), state.dog_face, state.dog_playing, mess};
}

State CleaningEnv::decode(InternalState s) const {
    std::optional<Pos> mess;
    if (s.mess >= 0) mess = cells[s.mess];
    return {cells[s.robot], cells[s.dog], s.face, s.playing, mess};
}

double CleaningEnv::reward(const State& state) const {
    return state.mess_pos ? -1.0 : 0.0;
}

std::map<State, double> CleaningEnv::next_state_probs(const State& state, int action) const {
    TransitionResult result = _transitions(encode(state), action);
    std::map<State, double> out;
    for (const auto& outcome : result.distribution) out[decode(outcome.state)] += outcome.probability;
    return out;
}

StepResult CleaningEnv::step(const State& state, int action, int seed) const {
    RNG rng = make_rng(seed);
    return step(state, action, rng);
}

StepResult CleaningEnv::step(const State& state, int action, RNG& rng) const {
    TransitionResult result = _transitions(encode(state), action);
    if (!result.start) {
        return {decode(_sample(result.distribution, uniform(rng))), result.event, {}};
    }
    InternalState current = *result.start;
    std::vector<State> path = {decode(current)};
    for (int i = 0; i < k; ++i) {
        WanderState w = _sample(_wander(current.dog, current.face, current.mess), uniform(rng));
        current = {current.robot, w.dog, w.face, false, w.mess};
        path.push_back(decode(current));
    }
    return {path.back(), result.event, path};
}

State CleaningEnv::next_state_sample(const State& state, int action, int seed) const {
    return step(state, action, seed).state;
}

State CleaningEnv::next_state_sample(const State& state, int action, RNG& rng) const {
    return step(state, action, rng).state;
}

State CleaningEnv::random_init(int seed) const {
    RNG rng = make_rng(seed);
    return random_init(rng);
}

State CleaningEnv::random_init(RNG& rng) const {
    std::uniform_int_distribution<int> cell(0, n_cells - 1);
    std::uniform_int_distribution<int> mess(-1, n_cells - 1);
    std::uniform_int_distribution<int> face(0, 3);
    int r = cell(rng), d = cell(rng), m = mess(rng), f = face(rng);
    return decode({r, d, f, false, m});
}

} // namespace cleaning_robot

