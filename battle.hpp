#pragma once
#include <iostream>
#include <string>
#include <vector>

struct spell {
    std::string name;
    int cost;
    int power;
};

struct item {
    std::string name;
    int quantity;
    int healthRestored;
    int magicRestored;
};

struct entity {
    std::string name;
    std::vector<spell> spells;
    std::vector<item> items;
    int health;
    int maxHealth;
    int mp;
    int maxMp;
    int offense;
    int defense;
};

enum class BATTLE_STATUS {
    WIN,
    LOSE,
    ONGOING
};

BATTLE_STATUS performRound(entity* hero, entity* enemy);
void delayAction(int duration);
