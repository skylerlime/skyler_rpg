#include "battle.hpp"
#include <random>
#include <chrono>
#include <thread>
#include <cmath>
#include <cctype>
#include <algorithm>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

std::mt19937 rng(std::random_device{}());

float randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

void delayAction(int duration) {
    this_thread::sleep_for(chrono::seconds(duration));
}

string format(string text) {
    for (char& c : text) {
        c = toupper(c);
    }
    return text;
}

int calculateAttackDamage(int attackerOffense, int attackedDefense) {
    if (attackedDefense <= 0) attackedDefense = 1;
    float ratio = static_cast<float>(attackerOffense) / attackedDefense;
    float baseDamage = 10.0f * pow(ratio, 1.2f);
    float variation = randomFloat(0.9f, 1.1f);
    baseDamage *= variation;
    bool isCritical = randomFloat(0.0f, 1.0f) < 0.15f;
    if (isCritical) {
        baseDamage *= 2.0f;
        cout << "Critical hit!\n";
    }
    if (baseDamage < 1) baseDamage = 1;
    if (baseDamage > 999) baseDamage = 999;
    return static_cast<int>(baseDamage);
}

int magic(int mp, vector<spell> spells) {
    if (mp == 0) {
        cout << "Out of MP!\n";
        return -1;
    }

    for (spell s : spells) {
        cout << s.name << " (Cost: " << s.cost << ", Power: " << s.power << ")\n";
    }

    cout << "\nWhich spell would you like to cast?\n";

    string playerSpellChoice;
    cin >> playerSpellChoice;
    playerSpellChoice = format(playerSpellChoice);

    int index = 0;
    for (spell possibleChoice : spells) {
        if (possibleChoice.name == playerSpellChoice) {
            if (possibleChoice.cost <= mp) {
                return index;
            }
            else {
                cout << "Not enough MP!\n";
                return -1;
            }
        }
        index++;
    }

    cout << "Invalid spell.\n";
    return -1;
}

void flee() {
    cout << "\n...\n";
    delayAction(2);
    cout << "What?\n";
    delayAction(2);
    cout << "Flee?\n";
    delayAction(2);
    cout << "Flee???\n";
    delayAction(2);
    cout << "Unbelieveable!\n";
    delayAction(2);
    cout << "You coward!\n";
    delayAction(2);
    cout << "I don't even know what to say...\n";
    delayAction(3);
    cout << "\nDragon: \"...\"\n";
    delayAction(2);
    cout << "Dragon: \"Well, this is awkward...\"\n";
    delayAction(2);
    cout << "Dragon: \"I'm just gonna... go...\"\n";
    delayAction(3);
    cout << "\n...\n";
    delayAction(2);
    cout << "...\n";
    delayAction(2);
    cout << "...\n";
    delayAction(2);
}

int itemSelection(vector<item> items) {
    if (items.empty()) {
    cout << "You're out of items!\n";
    return -1;
    }

    for (item i : items) {
        cout << i.name;
        if (i.healthRestored > 0) cout << " (Heals " << i.healthRestored << " HP";
        if (i.magicRestored > 0) cout << ", Restores " << i.magicRestored << "MP";
        cout << ")\n";
    }

    cout << "\nWhich item would you like to use?\n";

    string playerItemChoice;
    cin >> playerItemChoice;
    playerItemChoice = format(playerItemChoice);

    int index = 0;
    for (item possibleItemChoice : items) {
        if (possibleItemChoice.name == playerItemChoice) {
            return index;
        }
        index++;
    }

    cout << "Invalid item.\n";
    return -1;
}

BATTLE_STATUS performRound(entity* hero, entity* enemy) {
    delayAction(1);
    cout << "\nYour Status:\n" << hero->health << "/" << hero->maxHealth << " HP\n"
         << hero->mp << "/" << hero->maxMp << " MP\n";

    delayAction(1);
    cout << "\nEnemy Status:\n" << enemy->health << "/" << enemy->maxHealth << " HP\n"
         << enemy->mp << "/" << enemy->maxMp << " MP\n";

    delayAction(1);
    string playerInput;
    cout << "\nWhat is your move? (ATTACK, MAGIC, ITEM, FLEE): ";
    cin >> playerInput;
    playerInput = format(playerInput);

    if (playerInput == "ATTACK") {
        cout << "You swung your sword!\n";
        delayAction(1);
        int attackDamage = calculateAttackDamage(hero->offense, enemy->defense);
        cout << "The dragon took " << attackDamage << " damage!\n";
        enemy->health -= attackDamage;
    }

    else if (playerInput == "MAGIC") {
        int magicIndex = magic(hero->mp, hero->spells);
        if (magicIndex != -1) {
            spell chosenSpell = hero->spells.at(magicIndex);
            cout << "You casted " << chosenSpell.name << "!\n";
            int magicDamage = calculateAttackDamage(chosenSpell.power, enemy->defense);
            delayAction(1);
            cout << "The dragon took " << magicDamage << " damage!\n";
            enemy->health -= magicDamage;
            hero->mp -= chosenSpell.cost;
        }
        else {
            return BATTLE_STATUS::ONGOING;
        }
    }

    else if(playerInput == "ITEM") {
        int itemIndex = itemSelection(hero->items);
        if (itemIndex != -1) {
            item& chosenItem = hero->items.at(itemIndex);
            cout << "You used " << chosenItem.name << "!\n";
            delayAction(1);

            if (chosenItem.healthRestored > 0) {
                hero->health = min(hero->health + chosenItem.healthRestored, hero->maxHealth);
                cout << chosenItem.healthRestored << " HP restored!\n";
                delayAction(1);
            }

            if (chosenItem.magicRestored > 0) {
                hero->mp = min(hero->mp + chosenItem.magicRestored, hero->maxMp);
                cout << chosenItem.magicRestored << " MP restored!\n";
                delayAction(1);
            }

            if (--chosenItem.quantity == 0) {
                hero->items.erase(hero->items.begin() + itemIndex);
            }
        }
        else {
            return BATTLE_STATUS::ONGOING;
        }
    }

    else if (playerInput == "FLEE") {
        flee();
        return BATTLE_STATUS::LOSE;
    }
    
    // for debugging purposes
    else if (playerInput == "NOTHING") {
        
    }

    else {
        cout << "Invalid option!";
        return BATTLE_STATUS::ONGOING;
    }

    if (enemy->health <= 0) return BATTLE_STATUS::WIN;

    delayAction(1);
    int enemyChoice = rand() % 2; // 0 = attack, 1 = magic

    if (enemyChoice == 1 && !enemy->spells.empty()) {
        // choose a random spell the enemy can afford
        vector<spell> usableSpells;
        for (const spell& s : enemy->spells) {
            if (s.cost <= enemy->mp) usableSpells.push_back(s);
        }

        if (!usableSpells.empty()) {
            spell chosen = usableSpells[rand() % usableSpells.size()];
            int dmg = calculateAttackDamage(chosen.power, hero->defense);
            cout << "\nThe dragon casts " << chosen.name << "!\n";
            delayAction(1);
            cout << "You took " << dmg << " magic damage!\n";
            hero->health -= dmg;
            enemy->mp -= chosen.cost;
        } else {
            // fallback to basic attack if no usable spell
            cout << "\nThe dragon swings its claws at you!\n";
            delayAction(1);
            int dmg = calculateAttackDamage(enemy->offense, hero->defense);
            cout << "You took " << dmg << " damage!\n";
            hero->health -= dmg;
        }
    } else {
        // basic attack
        cout << "\nThe dragon swings its claws at you!\n";
        delayAction(1);
        int dmg = calculateAttackDamage(enemy->offense, hero->defense);
        cout << "You took " << dmg << " damage!\n";
        hero->health -= dmg;
    }

    // if hero died, end loop
    if (hero->health <= 0) return BATTLE_STATUS::LOSE;

    // hero and enemy are still alive
    return BATTLE_STATUS::ONGOING;
}