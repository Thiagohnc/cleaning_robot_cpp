#include "cleaning_robot.hpp"
#include <iostream>

using namespace cleaning_robot;

int main(int argc, char** argv) {
    CleaningEnv env(SMALL_ROOM, 0.2, 2);
    GreedyPolicy greedy(env);

    // Run with --animate to generate the same kind of GIF as the Python example.
    if (argc > 1 && std::string(argv[1]) == "--animate") {
        animate(greedy, env, 80, std::nullopt, 3, "greedy.gif");
        std::cout << "Saved greedy.gif\n";
        return 0;
    }

    TabularModel model(env);
    std::vector<int> policy = tabulate(model, greedy);
    std::vector<double> v = policy_evaluation(model, policy, 0.99);

    // NumPy abbreviates large arrays too.
    std::cout << '[' << v[0] << ' ' << v[1] << ' ' << v[2] << " ... "
              << v[model.n - 3] << ' ' << v[model.n - 2] << ' ' << v[model.n - 1] << "]\n";
}
