#include "battle.hpp"
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

int main() {
    std::cout << "Welcome to Super Skyler RPG: Birth By Fantasy Megami Tensei Quest.\n";
    delayAction(3);

    spell thunder = {"THUNDER", 10, 6};
    spell blizzard = {"BLIZZARD", 15, 9};
    spell fire = {"FIRE", 8, 7};
    item potion = {"POTION", 2, 50, 0};
    item ether = {"ETHER", 1, 0, 30};
    item elixir = {"ELIXIR", 1, 100, 100};

    entity hero = {
        "Player",
        {thunder, blizzard},
        {potion, ether, elixir},
        50, 50, 25, 25, 5, 5
    };

    entity enemy = {
        "Dragon",
        {fire},
        {},
        50, 50, 25, 25, 5, 5
    };

    BATTLE_STATUS status = BATTLE_STATUS::ONGOING;
    PlaySound(TEXT("music/battle.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

    std::cout << "\nA dragon appears!\n";
    delayAction(1);
    while (status == BATTLE_STATUS::ONGOING) {
        status = performRound(&hero, &enemy);
    }

    if (status == BATTLE_STATUS::WIN) {
        PlaySound(NULL, NULL, 0);
        PlaySound(TEXT("music/victory.wav"), NULL, SND_FILENAME | SND_ASYNC);
        std::cout << "\nThe dragon was defeated! You win!!\n";
        delayAction(6);
    }
    else if (status == BATTLE_STATUS::LOSE) {
        PlaySound(NULL, NULL, 0);
        PlaySound(TEXT("music/lose.wav"), NULL, SND_FILENAME | SND_ASYNC);
        std::cout << "\nYou lost consciousness! You lose...\n";
        delayAction(6);
    }

    return 0;
}
