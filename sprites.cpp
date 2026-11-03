// Sprite rows and palette transcribed verbatim from cleaning_robot/animate.py.
#include "animate.hpp"

namespace cleaning_robot {
const std::map<char, Color> PALETTE{
    {'k', {26, 28, 44}},
    {'w', {244, 244, 244}},
    {'l', {196, 204, 214}},
    {'g', {130, 140, 156}},
    {'d', {86, 94, 112}},
    {'b', {65, 166, 246}},
    {'c', {115, 239, 247}},
    {'r', {177, 62, 83}},
    {'p', {239, 125, 160}},
    {'o', {205, 130, 70}},
    {'O', {150, 85, 45}},
    {'t', {240, 205, 150}},
    {'y', {255, 205, 117}},
    {'Y', {230, 170, 60}},
    {'h', {255, 250, 200}},
};
const std::vector<std::string> ROBOT{
    "................",
    ".....kkkkkk.....",
    "...kkllllllkk...",
    "..klllwwwwlllk..",
    "..kllwwwwwwllk..",
    ".klwwwwwwwwwwlk.",
    ".klwwwkkkkwwwlk.",
    ".klwwkbccbkwwlk.",
    ".klwwkbbbbkwwlk.",
    ".klwwwkkkkwwwlk.",
    ".kglwwwwwwwwlgk.",
    "..kgllwwwwllgk..",
    "..kdggllllggdk..",
    "...kkddddddkk...",
    ".....kkkkkk.....",
    "................",
};
const std::vector<std::string> DOG_RIGHT{
    "................",
    "................",
    "..........kk....",
    ".........kOOk...",
    "........kooOokk.",
    "........kokooook",
    "..kk....koooootk",
    ".kOk....kotttkk.",
    "..kok...kooook..",
    "...kokkkkooook..",
    "...kooooooooOk..",
    "...kooootttook..",
    "...koOkkkkkoOk..",
    "...kok....kok...",
    "...kk.....kk....",
    "................",
};
const std::vector<std::string> DOG_DOWN{
    "................",
    "...kk......kk...",
    "..kOOk....kOOk..",
    "..kOokkkkkkoOk..",
    "...koooooooook..",
    "...kokooookok...",
    "...koootttook...",
    "....kotkktok....",
    ".....kttttk.....",
    "....kookkook....",
    "...kooooooook...",
    "...kootttook....",
    "...kooooooook...",
    "...kokkkkkkok...",
    "...kk......kk...",
    "................",
};
const std::vector<std::string> DOG_UP{
    "................",
    "...kk......kk...",
    "..kOOk....kOOk..",
    "..kOokkkkkkoOk..",
    "...koooooooook..",
    "...koooooooook..",
    "...koooooooook..",
    "....koooooook...",
    ".....kkooook....",
    "....kookkook....",
    "...koooOOoook...",
    "...kooOkkOook...",
    "...koooOOoook...",
    "...kokkkkkkok...",
    "...kk......kk...",
    "................",
};
const std::vector<std::string> PEE{
    "................",
    "................",
    "................",
    "................",
    "................",
    "......YYYY......",
    "....YYyyyyYY....",
    "...YyyhhyyyyY...",
    "..YyyhyyyyyyyY..",
    "..YyyyyyyyyyyY..",
    "...YyyyyyyyhyY..",
    "....YYyyyyyYY...",
    "......YYYYY.....",
    "................",
    "................",
    "................",
};
const std::vector<std::string> BUBBLE{
    "..kkkkkkkk..",
    ".kwwwwwwwwk.",
    ".kwwwrrwwwk.",
    ".kwwwrrwwwk.",
    ".kwwwrrwwwk.",
    ".kwwwrrwwwk.",
    ".kwwwwwwwwk.",
    ".kwwwrrwwwk.",
    ".kwwwwwwwwk.",
    "..kkkwkkkk..",
    "....kwk.....",
    ".....k......",
};
const std::vector<std::string> HEART{
    ".kk.kk.",
    "kppkrrk",
    "kpprrrk",
    "krrrrrk",
    ".krrrk.",
    "..krk..",
    "...k...",
};
Sprites make_sprites() {
    Sprites sprites{_sprite(ROBOT), _sprite(PEE), _sprite(BUBBLE), _sprite(HEART), {}};
    sprites.dog[E] = _sprite(DOG_RIGHT);
    sprites.dog[W] = Image(sprites.dog[E].width, sprites.dog[E].height, {0, 0, 0}, 0);
    for (int y = 0; y < sprites.dog[E].height; ++y) for (int x = 0; x < sprites.dog[E].width; ++x) {
        const auto p = sprites.dog[E].pixel(sprites.dog[E].width - 1 - x, y);
        sprites.dog[W].set(x, y, {p[0], p[1], p[2]}, p[3]);
    }
    sprites.dog[S] = _sprite(DOG_DOWN);
    sprites.dog[N] = _sprite(DOG_UP);
    return sprites;
}

const Sprites SPRITES = make_sprites();
} // namespace cleaning_robot
