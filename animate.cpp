#include "animate.hpp"
#include "third_party/gif.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace cleaning_robot {
namespace {
Color rgb(Pixel p) { return {p[0], p[1], p[2]}; }
Color shade(Pixel p, double factor) {
    return {std::uint8_t(p[0] * factor), std::uint8_t(p[1] * factor), std::uint8_t(p[2] * factor)};
}
void darken(Image& img, int x0, int y0, int x1, int y1, double factor) {
    for (int y = std::max(0, y0); y < std::min(img.height, y1); ++y)
        for (int x = std::max(0, x0); x < std::min(img.width, x1); ++x)
            img.set(x, y, shade(img.pixel(x, y), factor));
}
// Small built-in 3x5 bitmap font for the HUD, replacing Pillow's default font.
void draw_text(Image& image, const std::string& text, int x, int y) {
    static const std::map<char, std::string> font{
        {'0',"111101101101111"}, {'1',"010110010010111"}, {'2',"111001111100111"},
        {'3',"111001111001111"}, {'4',"101101111001001"}, {'5',"111100111001111"},
        {'6',"111100111101111"}, {'7',"111001010010010"}, {'8',"111101111101111"},
        {'9',"111101111001111"}, {'A',"010101111101101"}, {'B',"110101110101110"},
        {'C',"111100100100111"}, {'D',"110101101101110"}, {'E',"111100110100111"},
        {'F',"111100110100100"}, {'G',"111100101101111"}, {'H',"101101111101101"},
        {'I',"111010010010111"}, {'J',"001001001101111"}, {'K',"101101110101101"},
        {'L',"100100100100111"}, {'M',"101111111101101"}, {'N',"101111111111101"},
        {'O',"111101101101111"}, {'P',"111101111100100"}, {'Q',"111101101111001"},
        {'R',"110101110101101"}, {'S',"111100111001111"}, {'T',"111010010010010"},
        {'U',"101101101101111"}, {'V',"101101101101010"}, {'W',"101101111111101"},
        {'X',"101101010101101"}, {'Y',"101101010010010"}, {'Z',"111001010100111"},
        {'=',"000111000111000"}, {'-',"000000111000000"}, {'/',"001001010100100"},
        {'.',"000000000000010"}, {':',"000010000010000"}
    };
    for (unsigned char c : text) {
        auto it = font.find(static_cast<char>(std::toupper(c)));
        if (it != font.end()) for (int row = 0; row < 5; ++row) for (int col = 0; col < 3; ++col)
            if (it->second[row * 3 + col] == '1') image.set(x + col, y + row, {244, 244, 244});
        x += 4;
    }
}
FloatPos interpolate(Pos a, Pos b, double f) {
    return {a.first + f * (b.first - a.first), a.second + f * (b.second - a.second)};
}
}

Image::Image(int width_, int height_, Color color, std::uint8_t alpha) : width(width_), height(height_) {
    data.resize(std::size_t(width) * height * 4);
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) set(x, y, color, alpha);
}
Pixel Image::pixel(int x, int y) const {
    const auto i = (std::size_t(y) * width + x) * 4;
    return {data[i], data[i + 1], data[i + 2], data[i + 3]};
}
void Image::set(int x, int y, Color color, std::uint8_t alpha) {
    if (x < 0 || y < 0 || x >= width || y >= height) return;
    const auto i = (std::size_t(y) * width + x) * 4;
    std::copy(color.begin(), color.end(), data.begin() + static_cast<std::ptrdiff_t>(i));
    data[i + 3] = alpha;
}
void Image::fill(int x0, int y0, int x1, int y1, Color color) {
    for (int y = std::max(0, y0); y < std::min(height, y1); ++y)
        for (int x = std::max(0, x0); x < std::min(width, x1); ++x) set(x, y, color);
}
Image Image::resize(int new_width, int new_height) const {
    Image out(new_width, new_height);
    for (int y = 0; y < new_height; ++y) for (int x = 0; x < new_width; ++x) {
        const auto p = pixel(int(std::int64_t(x) * width / new_width), int(std::int64_t(y) * height / new_height));
        out.set(x, y, rgb(p), p[3]);
    }
    return out;
}
void Image::save_ppm(const std::string& filename) const {
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << width << ' ' << height << "\n255\n";
    for (std::size_t i = 0; i < data.size(); i += 4) out.write(reinterpret_cast<const char*>(data.data() + i), 3);
}
Image _sprite(const std::vector<std::string>& rows) {
    Image out(int(rows.front().size()), int(rows.size()), {0, 0, 0}, 0);
    for (int y = 0; y < out.height; ++y) {
        for (int x = 0; x < out.width; ++x) if (rows[y][x] != '.') out.set(x, y, PALETTE.at(rows[y][x]));
    }
    return out;
}
Image _floor_tile(int x, int y) {
    Image tile(T, T, (x + y) % 2 ? Color{193, 140, 96} : Color{186, 133, 90});
    for (int r = 0; r < T; r += 4) tile.fill(0, r, T, r + 1, {150, 100, 66});
    const int off = y % 2 ? 0 : 8;
    for (int r = 0; r < T; r += 4) tile.fill((off + 4 * (r / 4) * 3) % T, r, (off + 4 * (r / 4) * 3) % T + 1, r + 4, {150, 100, 66});
    for (int r = 1; r < T; r += 4) for (int c = 0; c < T; ++c) {
        auto p = tile.pixel(c, r);
        tile.set(c, r, {std::uint8_t(std::min(int(p[0]) + 14, 255)), std::uint8_t(std::min(int(p[1]) + 14, 255)),
                        std::uint8_t(std::min(int(p[2]) + 14, 255))});
    }
    return tile;
}
Image _garden(int height, int width, int seed) {
    auto rng = make_rng(seed);
    Image g(width, height, {106, 190, 48});
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x)
        if (uniform(rng) < 0.12) g.set(x, y, {88, 168, 40});
    std::uniform_int_distribution<int> ys(1, height - 2), xs(1, width - 2);
    for (int i = 0; i < height * width / 40; ++i) {
        const int y = ys(rng), x = xs(rng);
        for (auto [dx, dy] : {Pos{-1, 0}, Pos{1, 0}, Pos{0, 1}}) g.set(x + dx, y + dy, {60, 135, 40});
    }
    const std::array<Color, 4> colors{{{240, 90, 100}, {255, 220, 90}, {250, 250, 250}, {180, 130, 230}}};
    for (int i = 0; i < height * width / 160; ++i) {
        const int y = ys(rng), x = xs(rng);
        const auto color = colors[std::uniform_int_distribution<int>(0, 3)(rng)];
        for (auto [dx, dy] : DIRS) g.set(x + dx, y + dy, color);
        g.set(x, y, {250, 200, 60});
    }
    return g;
}
void _window(Image& canvas, int x, int y) {
    canvas.fill(x, y, x + 12, y + 11, OUTLINE);
    canvas.fill(x + 1, y + 1, x + 11, y + 10, {240, 236, 220});
    for (int i = 0; i < 7; ++i) {
        const double t = i / 6.0;
        Color color{std::uint8_t((1 - t) * 170 + t * 100), std::uint8_t((1 - t) * 205 + t * 135),
                    std::uint8_t((1 - t) * 230 + t * 175)};
        canvas.fill(x + 2, y + 2 + i, x + 10, y + 3 + i, color);
        for (int j : {i - 1, i + 3}) if (j >= 0 && 7 - j >= 0 && 7 - j < 8)
            canvas.set(x + 2 + 7 - j, y + 2 + i, {235, 245, 255});
    }
    canvas.fill(x + 5, y + 2, x + 7, y + 9, {240, 236, 220});
    canvas.fill(x + 2, y + 5, x + 10, y + 6, {240, 236, 220});
    canvas.fill(x - 1, y + 10, x + 13, y + 11, {200, 190, 170});
}
void _door_and_walkway(Image& canvas, int x, int y_face, int y_top) {
    canvas.fill(x - 1, 0, x + 15, y_top, {196, 186, 166});
    for (int r = 0; r < y_top; ++r) {
        if (r % 4 == 3) canvas.fill(x - 1, r, x + 15, r + 1, {150, 140, 124});
        else for (int c = (r / 4) % 2 ? 2 : 6; c < 16; c += 8) canvas.set(x - 1 + c, r, {150, 140, 124});
        canvas.set(x - 1, r, {120, 112, 100}); canvas.set(x + 14, r, {120, 112, 100});
    }
    canvas.fill(x, y_face, x + 14, y_face + FACE, OUTLINE);
    canvas.fill(x + 1, y_face + 1, x + 13, y_face + FACE, {178, 70, 62});
    for (int r : {2, 9}) {
        canvas.fill(x + 3, y_face + r, x + 11, y_face + r + 5, {150, 54, 50});
        canvas.fill(x + 3, y_face + r, x + 4, y_face + r + 5, {205, 100, 88});
    }
    canvas.fill(x + 10, y_face + 8, x + 12, y_face + 10, {250, 210, 80});
    canvas.fill(x - 1, y_face + FACE - 1, x + 15, y_face + FACE, {200, 190, 170});
}
void _wall_face(Image& face, Color paper, Color stripe) {
    face.fill(0, 0, face.width, face.height, paper);
    for (int x = 0; x < face.width; x += 4) face.fill(x, 0, x + 1, face.height, stripe);
    face.fill(0, face.height - 7, face.width, face.height, {150, 96, 64});
    face.fill(0, face.height - 7, face.width, face.height - 6, {196, 140, 96});
    face.fill(0, face.height - 3, face.width, face.height, {110, 66, 44});
}
Image _tiles(int height, int width, Color a, Color b, int size) {
    Image out(width, height);
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x)
        out.set(x, y, ((y / size + x / size) % 2 == 0) ? a : b);
    return out;
}
// Rectangle slicing helpers for the house background.
void carve(Image& canvas, int y0, int y1, int x0, int x1) {
    canvas.fill(std::max(x0 - 1, 0), std::max(y0 - 1, 0), x1 + 1, y1 + 1, OUTLINE);
}

void wall(Image& canvas, int y0, int y1, int x0, int x1, Color paper, Color stripe) {
    carve(canvas, y0, y1, x0, x1);
    Image face(x1 - x0, y1 - y0);
    _wall_face(face, paper, stripe);
    _blit(canvas, face, x0, y0);
}

HouseBackground _house(const Room& room) {
    const int w = room.width * T, h = room.height * T;
    const int ox = GARDEN + WALL, oy = GARDEN + WALL + FACE;
    const int height = oy + h + WALL + SIDING + GARDEN, width = ox + w + WALL + GARDEN;
    Image canvas = _garden(height, width);
    const int x_east = ox + w + WALL, y_north = oy - FACE - WALL, y_south = oy + h;
    darken(canvas, x_east, y_north + 3, x_east + 3, height, 0.7);
    canvas.fill(0, y_north, x_east, height, OUTLINE);
    canvas.fill(0, y_north + 1, x_east - 1, height, {222, 210, 182});
    canvas.fill(0, y_north + 1, x_east - 1, y_north + 2, {240, 232, 210});
    wall(canvas, oy - FACE, oy, ox, ox + w, {236, 214, 190}, {222, 196, 170});
    carve(canvas, oy, y_south, ox, ox + w);
    const int xw = ox - WALL - 1;
    wall(canvas, oy - FACE, oy, 0, xw, {196, 214, 180}, {176, 198, 160});
    carve(canvas, oy, y_south, 0, xw);
    _blit(canvas, _tiles(y_south - oy, xw, {170, 196, 214}, {150, 178, 200}, 8), 0, oy);
    const int yd = oy + (room.height / 2) * T + 1;
    canvas.fill(xw, yd, ox, yd + 14, OUTLINE);
    canvas.fill(xw + 1, yd + 1, ox - 1, yd + 13, {178, 70, 62});
    canvas.fill(xw + 2, yd + 1, xw + 3, yd + 13, {205, 100, 88});
    canvas.fill(ox - 3, yd + 6, ox - 1, yd + 8, {250, 210, 80});
    const int ys = y_south + WALL;
    wall(canvas, ys, ys + 11, 0, ox + w, {250, 236, 200}, {238, 220, 180});
    carve(canvas, ys + 11, height, 0, ox + w);
    _blit(canvas, _tiles(height - ys - 11, ox + w, {240, 236, 226}, {90, 120, 140}, 6), 0, ys + 11);
    const int door = room.width / 2;
    for (int i = 1; i < room.width; i += 3) if (i != door) _window(canvas, ox + i * T + 2, oy - FACE - 2);
    _door_and_walkway(canvas, ox + door * T + 1, oy - FACE, y_north);
    return {canvas, ox, oy};
}
Image _table_tile(const Image& floor, bool left, bool right, bool top, bool bottom, bool centerpiece) {
    const Color wood{176, 110, 62}, wood_dark{128, 74, 40};
    Image tile = floor;
    const int y0 = top ? 1 : 0, y1 = bottom ? 13 : 16, x0 = left ? 1 : 0, x1 = right ? 15 : 16;
    if (bottom) {
        darken(tile, x0, 13, x1, 16, 0.7);
        std::vector<int> legs;
        if (left) legs.push_back(x0 + 1);
        if (right) legs.push_back(x1 - 3);
        for (int lx : legs) { tile.fill(lx, 13, lx + 2, 16, wood_dark); tile.fill(lx, 15, lx + 2, 16, OUTLINE); }
    }
    tile.fill(x0, y0, x1, y1, OUTLINE);
    tile.fill(x0 + left, y0 + top, x1 - right, y1 - bottom, wood);
    tile.fill(x0 + left, y1 - bottom - 2, x1 - right, y1 - bottom, wood_dark);
    for (int y = 3; y < 11; ++y) for (int x = x0 + left; x < x1 - right; ++x)
        tile.set(x, y, ((y / 2 + x / 2) % 2) ? Color{210, 70, 80} : Color{240, 236, 228});
    if (centerpiece) {
        tile.fill(6, 5, 10, 10, OUTLINE); tile.fill(7, 6, 9, 9, {70, 120, 200});
        for (int y = 0; y < 6; ++y) for (int x = 0; x < T; ++x)
            if ((y - 6.5) * (y - 6.5) + (x - 7.5) * (x - 7.5) <= 3.5) tile.set(x, y, {90, 170, 90});
        tile.fill(5, 2, 7, 4, {240, 120, 160}); tile.fill(8, 1, 10, 3, {250, 220, 90});
        tile.fill(10, 3, 12, 5, {240, 120, 160});
    } else for (int y = 0; y < T; ++y) for (int x = 0; x < T; ++x) {
        const double r2 = (y - 6.5) * (y - 6.5) + (x - 7.5) * (x - 7.5);
        if (r2 <= 12) tile.set(x, y, {150, 150, 160});
        if (r2 <= 7) tile.set(x, y, {250, 250, 250});
        if (r2 <= 2) tile.set(x, y, {225, 225, 230});
    }
    return tile;
}
void _blit(Image& canvas, const Image& sprite, int px, int py) {
    for (int y = std::max(py, 0); y < std::min(py + sprite.height, canvas.height); ++y)
        for (int x = std::max(px, 0); x < std::min(px + sprite.width, canvas.width); ++x) {
            auto s = sprite.pixel(x - px, y - py), d = canvas.pixel(x, y);
            const double alpha = s[3] / 255.0;
            canvas.set(x, y, {std::uint8_t(alpha * s[0] + (1 - alpha) * d[0]),
                              std::uint8_t(alpha * s[1] + (1 - alpha) * d[1]),
                              std::uint8_t(alpha * s[2] + (1 - alpha) * d[2])});
        }
}
Renderer::Renderer(const CleaningEnv& env_, int hud_height) : env(env_), hud(hud_height) {
    HouseBackground house = _house(env.room);
    background = house.image;
    ox = house.ox;
    oy = house.oy;
    height = background.height + hud; width = background.width;
    const auto& obs = env.room.obstacles;
    for (int y = 0; y < env.room.height; ++y) for (int x = 0; x < env.room.width; ++x) {
        Image tile = _floor_tile(x, y);
        if (obs.count({x, y})) {
            std::vector<int> xs;
            for (auto [cx, cy] : obs) if (cy == y) xs.push_back(cx);
            std::sort(xs.begin(), xs.end());
            tile = _table_tile(tile, !obs.count({x - 1, y}), !obs.count({x + 1, y}),
                               !obs.count({x, y - 1}), !obs.count({x, y + 1}), x == xs[xs.size() / 2]);
        }
        _blit(background, tile, ox + x * T, oy + y * T);
    }
}
Pos Renderer::_px(FloatPos pos) const {
    // nearbyint uses round-to-even, matching Python's round at half-pixel ties.
    return {ox + int(std::nearbyint(pos.first * T)), oy + int(std::nearbyint(pos.second * T)) + hud};
}
Image Renderer::frame(const State& state, std::optional<FloatPos> robot_pos, std::optional<FloatPos> dog_pos,
                       const std::string& hud_text, int bob, bool show_view, bool reboot) const {
    Image canvas(width, height, OUTLINE); _blit(canvas, background, 0, hud);
    if (show_view && !state.dog_playing) {
        const int dog = env.index.at(state.dog_pos);
        for (int c = 0; c < env.n_cells; ++c) if (env.sees[dog][state.dog_face][c]) {
            const auto [px, py] = _px(env.cells[c]);
            for (int y = py; y < py + T; ++y) for (int x = px; x < px + T; ++x) {
                auto p = canvas.pixel(x, y);
                canvas.set(x, y, {std::uint8_t(0.7 * p[0] + 0.3 * 255), std::uint8_t(0.7 * p[1] + 0.3 * 120),
                                  std::uint8_t(0.7 * p[2] + 0.3 * 190)});
            }
        }
    }
    if (state.mess_pos) { const auto [x, y] = _px(*state.mess_pos); _blit(canvas, SPRITES.pee, x, y); }
    const auto [rx, ry] = _px(robot_pos.value_or(FloatPos(state.robot_pos)));
    auto robot = SPRITES.robot;
    if (reboot) for (int y = 0; y < robot.height; ++y) for (int x = 0; x < robot.width; ++x) {
        const auto p = robot.pixel(x, y); robot.set(x, y, shade(p, 0.5), p[3]);
    }
    _blit(canvas, robot, rx, ry);
    const auto [dx, dy] = _px(dog_pos.value_or(FloatPos(state.dog_pos)));
    _blit(canvas, SPRITES.dog.at(state.dog_face), dx, dy - bob);
    if (state.dog_playing) _blit(canvas, SPRITES.bubble, dx + 2, dy - 12);
    if (reboot) _blit(canvas, SPRITES.heart, rx + 9, ry - 4);
    if (!hud_text.empty()) draw_text(canvas, hud_text, 3, 1);
    return canvas;
}
RolloutResult rollout(const CleaningEnv& env, const PolicyMixin& policy, int steps, std::optional<State> state, int seed) {
    auto rng = make_rng(seed); return rollout(env, policy, steps, state, rng);
}
RolloutResult rollout(const CleaningEnv& env, const PolicyMixin& policy, int steps, std::optional<State> state, RNG& rng) {
    // Avoid eagerly sampling random_init when an initial state was supplied.
    auto current = state ? *state : env.random_init(rng);
    std::vector<State> states{current}; std::vector<int> actions; std::vector<double> rewards;
    std::vector<std::string> events; std::vector<std::vector<State>> reboots;
    for (int t = 0; t < steps; ++t) {
        const int a = policy.act(current, rng); rewards.push_back(env.reward(current));
        auto [next, event, path] = env.step(current, a, rng);
        current = next; states.push_back(current); actions.push_back(a);
        events.push_back(event); reboots.push_back(std::move(path));
    }
    return {states, actions, rewards, events, reboots};
}
std::vector<Image> animate(const PolicyMixin& policy, const CleaningEnv& env, int steps, std::optional<State> state,
                           int seed, const std::string& filename, int tween, int frame_ms, int scale, bool show_view) {
    Renderer renderer(env);
    auto [states, actions, rewards, events, reboots] = rollout(env, policy, steps, state, seed);
    std::vector<Image> frames;
    double total = 0; int captures = 0;
    for (int t = 0; t < steps; ++t) {
        const auto& s0 = states[t]; const auto& s1 = states[t + 1];
        total += rewards[t]; const bool captured = events[t] == "captured";
        captures += captured;
        std::ostringstream hud;
        hud << "t=" << std::setw(3) << t + 1 << " R=" << std::setw(5) << std::fixed << std::setprecision(0) << total
            << " k=" << env.k << " caught=" << captures;
        if (captured) {
            const auto caught_at = _caught_at(env, s0, actions[t]); auto chasing = s0; chasing.dog_playing = true;
            for (int i = 0; i < tween; ++i) {
                const double f = double(i + 1) / tween;
                frames.push_back(renderer.frame(chasing, interpolate(s0.robot_pos, caught_at, f),
                                 interpolate(s0.dog_pos, caught_at, f), hud.str(), 0, show_view));
            }
            const auto& path = reboots[t];
            for (int i = 0; i < tween; ++i) frames.push_back(renderer.frame(path[0], std::nullopt, std::nullopt,
                "REBOOT 0/" + std::to_string(env.k), 0, show_view, true));
            for (std::size_t j = 1; j < path.size(); ++j) _tween_round(frames, renderer, path[j - 1], path[j], tween,
                "REBOOT " + std::to_string(j) + "/" + std::to_string(env.k), show_view, true);
        } else _tween_round(frames, renderer, s0, s1, tween, hud.str(), show_view);
    }
    for (auto& frame : frames) {
        if (scale != 1) frame = frame.resize(frame.width * scale, frame.height * scale);
    }
    if (!filename.empty()) save_gif(frames, filename, frame_ms);
    return frames;
}
void _tween_round(std::vector<Image>& frames, const Renderer& renderer, const State& s0, const State& s1,
                  int tween, const std::string& hud, bool show_view, bool reboot) {
    for (int i = 0; i < tween; ++i) {
        const double f = double(i + 1) / tween;
        auto shown = s1;
        if (i != tween - 1) { shown.robot_pos = s0.robot_pos; shown.dog_pos = s0.dog_pos; shown.mess_pos = s0.mess_pos; }
        const int bob = i % 2 && s0.dog_pos != s1.dog_pos ? 1 : 0;
        frames.push_back(renderer.frame(shown, interpolate(s0.robot_pos, s1.robot_pos, f),
                         interpolate(s0.dog_pos, s1.dog_pos, f), hud, bob, show_view, reboot));
    }
}
Pos _caught_at(const CleaningEnv& env, const State& state, int action) {
    if (action >= 0 && action < 4) {
        const Pos next{state.robot_pos.first + DIRS[action].first, state.robot_pos.second + DIRS[action].second};
        if (env.room.is_free(next)) return next;
    }
    return state.robot_pos;
}
void save_gif(const std::vector<Image>& frames, const std::string& filename, int frame_ms) {
    const Image& first = frames[0];
    int delay = frame_ms / 10; // GIF durations use hundredths of a second.
    GifWriter writer{};
    GifBegin(&writer, filename.c_str(), first.width, first.height, delay);
    for (const Image& frame : frames) {
        GifWriteFrame(&writer, frame.data.data(), frame.width, frame.height, delay);
    }
    GifEnd(&writer);
}

} // namespace cleaning_robot
