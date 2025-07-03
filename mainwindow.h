#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>
#include <QPixmap>
#include <QMovie>
#include <QTimer>
#include <QListWidgetItem>
#include <random>
#include <cmath>
#include <string>
#include <vector>
#include <utility>

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

enum class TURN {
    PLAYER,
    ENEMY
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    QSoundEffect *attackSfx;
    QSoundEffect *spellSfx;
    QSoundEffect *selectSfx;
    QSoundEffect *itemSfx;
    QSoundEffect *critSfx;
    BATTLE_STATUS status;
    entity hero;
    entity enemy;
    QString currentMenu;
    QString currentNarration;
    bool playerTurn;

    void updateUI();
    void showMainMenu();
    void showNarrationOnly(const QString& message, std::function<void()> onComplete);
    void printTextScroll(const QString& message, std::function<void()> onComplete = nullptr);
    void showSelectionMenu(const QString& menu);
    float randomFloat(float min, float max);
    std::pair<int, bool> calculateAttackDamage(int attackerOffense, int attackedDefense);
    void playMusic(const QString& resourcePath);
    void enemyTurn();
    void battleOver();

private slots:
    void attackClicked();
    void magicClicked();
    void itemClicked();
    void fleeClicked();
    void selectionClicked(QListWidgetItem* item);
};

#endif // MAINWINDOW_H
