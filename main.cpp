#include "Game.h"
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    Game game;
    game.run();
    return 0;

}
