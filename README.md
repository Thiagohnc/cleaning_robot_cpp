# Cleaning robot in C++

C++17 translation of `../cleaning_robot`. Each Python module has a matching
`.hpp` / `.cpp` pair. `cleaning_robot.hpp` includes all the public headers.
The implementation uses ordinary structs, standard containers, and loops;
there are no custom templates or added input validation.

## Build and run

From the parent `exercicio1` directory:

```bash
cmake -S cleaning_robot_cpp -B cleaning_robot_cpp/build -DCMAKE_BUILD_TYPE=Release
cmake --build cleaning_robot_cpp/build
./cleaning_robot_cpp/build/cleaning_robot
```

The default program follows `main.py`: build the default room's model,
tabulate `GreedyPolicy`, and evaluate it with `gamma = 0.99`.
The default model has 606,816 states and 11,512,884 sparse transition entries.

To generate an animation in the current working directory:

```bash
./cleaning_robot_cpp/build/cleaning_robot --animate
```

## Using the code

```cpp
#include "cleaning_robot.hpp"
using namespace cleaning_robot;

CleaningEnv env(SMALL_ROOM, 0.2, 2);
State state = env.random_init(3);
double reward = env.reward(state);
auto probabilities = env.next_state_probs(state, E);
State next = env.next_state_sample(state, E, 3);

GreedyPolicy greedy(env);
auto frames = animate(greedy, env, 80, std::nullopt, 3, "greedy.gif");

TabularModel model(env);
auto policy = tabulate(model, greedy);
auto values = policy_evaluation(model, policy, 0.99);
```

`SparseMatrix` uses compressed sparse row storage: `indptr` gives row offsets,
`indices` gives successor state indices, and `data` gives probabilities.
`model.P[a](i, j)` returns a single probability. The algorithms iterate over
stored entries directly; no dense transition matrix is allocated.

`all_states` returns a vector in the Python enumeration order. `state_at` visits
a state by index without constructing that vector. `step` returns a struct with
`state`, `event`, and `reboot` fields; `rollout` returns a struct with `states`,
`actions`, `rewards`, `events`, and `reboots` fields. An empty event string means
Python's `None`; optional positions use `std::nullopt`.

The default seed `-1` selects a random seed. Pass an integer for repeatability,
or an `RNG&` to share a random stream between calls. C++ and NumPy use different
generators, so the same seed does not produce the same sampled trajectory.
Keep the environment alive while using its model, policies, or renderer.

## Original unfinished code

`ValueIteration`, `PolicyIteration`, and `QLearning` remain exercise placeholders.
Their exceptions correspond to the Python source's `NotImplementedError`.
Stochastic policy evaluation is still a TODO.

The deterministic `policy_evaluation` restores the value assignment commented
out in Python and stops when the largest update is below `1e-8`. Without those
two changes, the Python loop never updates its values or returns. It preserves
the source's `R[next_s] + gamma * v[next_s]` reward convention.
`PolicyIteration._learn` retains its unfinished loop from Python.

## Included library

`third_party/gif.h` and `third_party/LICENSE.gif-h` were downloaded from
[gif-h](https://github.com/charlietangora/gif-h). They provide GIF encoding;
no Python runtime, package installation, or build-time download is needed.
Sparse matrices and pixel drawing are implemented locally. The original sprite
rows and room artwork are retained; the HUD uses a small built-in bitmap font.
