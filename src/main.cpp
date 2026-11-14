#include "Game.hpp"

#if defined(__ANDROID__)
__attribute__((visibility("default")))
#endif
int main(int argc, char* argv[]) {
    Game game;
    game.run();
    return 0;
}
