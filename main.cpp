#include "cleaning_robot.hpp"
#include <iostream>
#include <fstream>

using namespace cleaning_robot;
using namespace std;

void question1(void) {
    const double p = 0.2;

    for(int k: {0, 1, 2, 4, 6, 10}) {
        for(double gamma: {0.7, 0.99}) {
            CleaningEnv env(SMALL_ROOM, p, k);
            GreedyPolicy greedy_policy(env);
            TabularModel model(env);
            std::vector<int> policy = tabulate(model, greedy_policy);
            cout << "Gulosa - k = " << k << ", gamma = " << gamma << ": ";
            std::vector<double> v = policy_evaluation(model, policy, gamma);
            ofstream outfile;
            outfile.open("outputs/q1/greedy_" + to_string(k) + "_" + to_string(gamma));
            for(double val: v) {
                outfile << val << '\n';
            }
            outfile.close();
        }
    }

    for(int k: {0, 1, 2, 4, 6, 10}) {
        for(double gamma: {0.7, 0.99}) {
            CleaningEnv env(SMALL_ROOM, p, k);
            RandomPolicy random_policy;
            TabularModel model(env);
            std::vector<int> policy = tabulate(model, random_policy);
            cout << "Aleatória - k = " << k << ", gamma = " << gamma << ": ";
            std::vector<double> v = policy_evaluation(model, policy, gamma);
            ofstream outfile;
            outfile.open("outputs/q1/random_" + to_string(k) + "_" + to_string(gamma));
            for(double val: v) {
                outfile << val << '\n';
            }
            outfile.close();
        }
    }
}

int main(int argc, char** argv) {
    question1();
    exit(0);

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
