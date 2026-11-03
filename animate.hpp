#pragma once
#include "env.hpp"
#include "policy.hpp"

namespace cleaning_robot {
inline constexpr int T = 16, WALL = 5, FACE = T, SIDING = 7, GARDEN = 18;
using Color = std::array<std::uint8_t, 3>;
using Pixel = std::array<std::uint8_t, 4>;
using FloatPos = std::pair<double, double>;
inline constexpr Color OUTLINE{26, 28, 44};

// RGB/RGBA array and Pillow Image replacement, with nearest-neighbor resizing.
struct Image {
    int width = 0, height = 0;
    std::vector<std::uint8_t> data;
    Image() = default;
    Image(int width, int height, Color color = {0, 0, 0}, std::uint8_t alpha = 255);
    Pixel pixel(int x, int y) const;
    void set(int x, int y, Color color, std::uint8_t alpha = 255);
    void fill(int x0, int y0, int x1, int y1, Color color);
    Image resize(int new_width, int new_height) const;
    void save_ppm(const std::string& filename) const;
};
extern const std::map<char, Color> PALETTE;
extern const std::vector<std::string> ROBOT, DOG_RIGHT, DOG_DOWN, DOG_UP, PEE, BUBBLE, HEART;
Image _sprite(const std::vector<std::string>& rows);
struct Sprites {
    Image robot, pee, bubble, heart;
    std::array<Image, 4> dog;
};
extern const Sprites SPRITES;
Image _floor_tile(int x, int y);
Image _garden(int height, int width, int seed = 0);
void _window(Image& canvas, int x, int y);
void _door_and_walkway(Image& canvas, int x, int y_face, int y_top);
void _wall_face(Image& face, Color paper, Color stripe);
Image _tiles(int height, int width, Color a, Color b, int size);
struct HouseBackground {
    Image image;
    int ox, oy;
};
HouseBackground _house(const Room& room);
Image _table_tile(const Image& floor, bool left, bool right, bool top, bool bottom, bool centerpiece);
void _blit(Image& canvas, const Image& sprite, int px, int py);

class Renderer {
public:
    const CleaningEnv& env;
    int hud, ox, oy, height, width;
    Image background;
    explicit Renderer(const CleaningEnv& env, int hud_height = 12);
    Pos _px(FloatPos pos) const;
    Image frame(const State& state, std::optional<FloatPos> robot_pos = std::nullopt,
                std::optional<FloatPos> dog_pos = std::nullopt, const std::string& hud = "",
                int bob = 0, bool show_view = true, bool reboot = false) const;
};
struct RolloutResult {
    std::vector<State> states;
    std::vector<int> actions;
    std::vector<double> rewards;
    std::vector<std::string> events;
    std::vector<std::vector<State>> reboots;
};
RolloutResult rollout(const CleaningEnv& env, const PolicyMixin& policy, int steps,
                      std::optional<State> state = std::nullopt, int seed = -1);
RolloutResult rollout(const CleaningEnv& env, const PolicyMixin& policy, int steps, std::optional<State> state, RNG& rng);
std::vector<Image> animate(const PolicyMixin& policy, const CleaningEnv& env, int steps = 100,
                           std::optional<State> state = std::nullopt, int seed = -1,
                           const std::string& filename = "", int tween = 4,
                           int frame_ms = 60, int scale = 3, bool show_view = true);
void _tween_round(std::vector<Image>& frames, const Renderer& renderer, const State& s0, const State& s1,
                  int tween, const std::string& hud, bool show_view, bool reboot = false);
Pos _caught_at(const CleaningEnv& env, const State& state, int action);
void save_gif(const std::vector<Image>& frames, const std::string& filename, int frame_ms = 60);
} // namespace cleaning_robot
