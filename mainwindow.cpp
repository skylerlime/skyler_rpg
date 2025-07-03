#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , status(BATTLE_STATUS::ONGOING)
{
    ui->setupUi(this);
    setFixedSize(size());

    auto* backgroundMovie = new QMovie(":/assets/sprites/hyzfbno2nmmc1.gif");
    ui->backgroundLabel->setMovie(backgroundMovie);
    backgroundMovie->start();

    ui->heroPortrait->setPixmap(QPixmap(":/assets/sprites/hero_stats.png"));
    ui->commandMenu->setPixmap(QPixmap(":/assets/sprites/command_menu.png"));
    ui->dragon->setPixmap(QPixmap(":/assets/sprites/dragon.png"));

    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    playMusic("qrc:/assets/music/battle.wav");

    attackSfx = new QSoundEffect(this);
    attackSfx->setSource(QUrl("qrc:/assets/music/hit.wav"));
    attackSfx->setVolume(0.8f);

    critSfx = new QSoundEffect(this);
    critSfx->setSource(QUrl("qrc:/assets/music/crit.wav"));
    critSfx->setVolume(0.8f);

    itemSfx = new QSoundEffect(this);
    itemSfx->setSource(QUrl("qrc:/assets/music/item.wav"));
    itemSfx->setVolume(0.8f);

    spellSfx = new QSoundEffect(this);
    spellSfx->setSource(QUrl("qrc:/assets/music/spell.wav"));
    spellSfx->setVolume(0.8f);

    selectSfx = new QSoundEffect(this);
    selectSfx->setSource(QUrl("qrc:/assets/music/select.wav"));
    selectSfx->setVolume(0.8f);

    connect(ui->attackButton, &QPushButton::clicked, this, &MainWindow::attackClicked);
    connect(ui->magicButton, &QPushButton::clicked, this, &MainWindow::magicClicked);
    connect(ui->itemButton, &QPushButton::clicked, this, &MainWindow::itemClicked);
    connect(ui->fleeButton, &QPushButton::clicked, this, &MainWindow::fleeClicked);
    connect(ui->selectionList, &QListWidget::itemClicked, this, &MainWindow::selectionClicked);

    hero = {"Player",
            {{"THUNDER", 10, 6}, {"BLIZZARD", 15, 9}},
            {{"POTION", 2, 50, 0}, {"ETHER", 1, 0, 30}, {"ELIXIR", 1, 100, 100}},
            50, 50, 25, 25, 5, 5};

    enemy = {"Dragon", {{"FIRE", 8, 7}}, {}, 50, 50, 25, 25, 5, 5};

    playerTurn = true;
    updateUI();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateUI() {
    ui->HPLabel->setText(QString("HP:%1/%2").arg(hero.health).arg(hero.maxHealth));
    ui->MPLabel->setText(QString("MP:%1/%2").arg(hero.mp).arg(hero.maxMp));

    const bool canAct = (status == BATTLE_STATUS::ONGOING) && playerTurn;
    ui->attackButton->setVisible(canAct);
    ui->magicButton->setVisible(canAct);
    ui->itemButton->setVisible(canAct && !hero.items.empty());
    ui->fleeButton->setVisible(canAct);
    ui->selectionList->setVisible(false);
}

void MainWindow::showMainMenu() {
    ui->attackButton->show();
    ui->magicButton->show();
    ui->itemButton->show();
    ui->fleeButton->show();
    ui->selectionList->hide();
    ui->messageLabel->clear();
}

void MainWindow::showNarrationOnly(const QString& message, std::function<void()> onComplete) {
    ui->attackButton->hide();
    ui->magicButton->hide();
    ui->itemButton->hide();
    ui->fleeButton->hide();
    ui->selectionList->hide();

    printTextScroll(message, onComplete);
}

void MainWindow::printTextScroll(const QString& message, std::function<void()> onComplete) {
    currentNarration.clear();
    ui->messageLabel->clear();

    static QTimer* scrollTimer = nullptr;
    int* index = new int(0);

    if (scrollTimer) {
        scrollTimer->stop();
        delete scrollTimer;
    }

    scrollTimer = new QTimer(this);
    scrollTimer->setInterval(25);

    connect(scrollTimer, &QTimer::timeout, this, [=]() mutable {
        if (*index < message.length()) {
            currentNarration += message[*index];
            ui->messageLabel->setText(currentNarration);
            (*index)++;
        } else {
            scrollTimer->stop();
            delete index;
            if (onComplete) QTimer::singleShot(2000, this, onComplete);
        }
    });

    scrollTimer->start();
}

void MainWindow::showSelectionMenu(const QString& menu) {
    ui->selectionList->clear();

    if (menu == "spells") {
        for (const auto& s : hero.spells) {
            ui->selectionList->addItem(QString::fromStdString(s.name) + QString(" (Cost: %1 MP)").arg(s.cost));
        }
    } else if (menu == "items") {
        for (const auto& i : hero.items) {
            QString itemStr = QString::fromStdString(i.name) + QString(" (x%1)").arg(i.quantity);
            if (i.healthRestored > 0) itemStr += QString(", +%1 HP").arg(i.healthRestored);
            if (i.magicRestored > 0) itemStr += QString(", +%1 MP").arg(i.magicRestored);
            ui->selectionList->addItem(itemStr);
        }
    }
    ui->selectionList->addItem(QString::fromStdString("RETURN"));

    ui->selectionList->show();
    ui->attackButton->hide();
    ui->magicButton->hide();
    ui->itemButton->hide();
    ui->fleeButton->hide();

    currentMenu = menu;
}

void MainWindow::attackClicked() {
    if (status != BATTLE_STATUS::ONGOING) return;

    playerTurn = false;
    updateUI();

    auto [dmg, isCrit] = calculateAttackDamage(hero.offense, enemy.defense);
    enemy.health -= dmg;

    isCrit ? critSfx->play() : attackSfx->play();

    showNarrationOnly(
    isCrit
        ? QString("Critical hit! You dealt %1 damage!").arg(dmg)
        : QString("You dealt %1 damage!").arg(dmg),
    [this]() {
        if (enemy.health <= 0) {
            status = BATTLE_STATUS::WIN;
            battleOver();
        } else {
            QTimer::singleShot(500, this, &MainWindow::enemyTurn);
        }
        updateUI();
    });
}

void MainWindow::magicClicked() {
    if (status != BATTLE_STATUS::ONGOING || hero.spells.empty()) return;

    selectSfx->play();
    
    ui->selectionList->clear();
    for (const auto& s : hero.spells) {
        ui->selectionList->addItem(QString::fromStdString(s.name) + QString(" (Cost: %1 MP)").arg(s.cost));
    }
    ui->selectionList->addItem("Return");
    showSelectionMenu("spells");
}

void MainWindow::itemClicked() {
    if (status != BATTLE_STATUS::ONGOING || hero.items.empty()) return;

    selectSfx->play();

    ui->selectionList->clear();
    for (const auto& i : hero.items) {
        QString itemStr = QString::fromStdString(i.name) + QString(" (x%1)").arg(i.quantity);
        if (i.healthRestored > 0) itemStr += QString(", Heals %1 HP").arg(i.healthRestored);
        if (i.magicRestored > 0) itemStr += QString(", Restores %1 MP").arg(i.magicRestored);
        ui->selectionList->addItem(itemStr);
    }
    showSelectionMenu("items");
}

void MainWindow::selectionClicked(QListWidgetItem* item) {
    if (item->text() == "RETURN") {
        showMainMenu();
        return;
    }

    int index = ui->selectionList->row(item);

    if (currentMenu == "spells") {
        const spell& chosenSpell = hero.spells[index];
        if (chosenSpell.cost > hero.mp) {
            showNarrationOnly("Not enough MP!", [this]() {
                showMainMenu();
                updateUI();
            });
            return;
        }

        playerTurn = false;
        updateUI();

        auto [dmg, isCrit] = calculateAttackDamage(chosenSpell.power, enemy.defense);
        enemy.health -= dmg;
        hero.mp -= chosenSpell.cost;
        spellSfx->play();

        showNarrationOnly(
            isCrit ? QString("Critical spell! %1 deals %2 damage!").arg(QString::fromStdString(chosenSpell.name)).arg(dmg)
                   : QString("You cast %1 and dealt %2 damage!").arg(QString::fromStdString(chosenSpell.name)).arg(dmg),
            [this]() {
                if (enemy.health <= 0) {
                    status = BATTLE_STATUS::WIN;
                    battleOver();
                } else {
                    QTimer::singleShot(500, this, &MainWindow::enemyTurn);
                }
                updateUI();
            });

    } else if (currentMenu == "items") {
        ::item& chosenItem = hero.items[index];
        QString msg = QString("You used %1!").arg(QString::fromStdString(chosenItem.name));

        if (chosenItem.healthRestored > 0) {
            hero.health = std::min(hero.health + chosenItem.healthRestored, hero.maxHealth);
            msg += QString("\nRestored %1 HP.").arg(chosenItem.healthRestored);
        }
        if (chosenItem.magicRestored > 0) {
            hero.mp = std::min(hero.mp + chosenItem.magicRestored, hero.maxMp);
            msg += QString("\nRestored %1 MP.").arg(chosenItem.magicRestored);
        }

        chosenItem.quantity--;
        if (chosenItem.quantity <= 0)
            hero.items.erase(hero.items.begin() + index);

        playerTurn = false;
        updateUI();
        itemSfx->play();

        showNarrationOnly(msg, [this]() {
            QTimer::singleShot(500, this, &MainWindow::enemyTurn);
        });
    }
}

void MainWindow::fleeClicked() {
    selectSfx->play();
    status = BATTLE_STATUS::LOSE;

    showNarrationOnly("You fled from battle... for some reason.", [this]() {
        QTimer::singleShot(5000, this, []() {
            QApplication::quit();
        });
    });
}

void MainWindow::enemyTurn() {
    if (enemy.health <= 0 || hero.health <= 0) return;

    bool usedSpell = false;
    QString message;
    if (!enemy.spells.empty() && (rand() % 2) == 1) {
        std::vector<spell> usableSpells;
        for (const auto& s : enemy.spells)
            if (s.cost <= enemy.mp)
                usableSpells.push_back(s);

        if (!usableSpells.empty()) {
            const spell& chosen = usableSpells[rand() % usableSpells.size()];
            auto [dmg, isCrit] = calculateAttackDamage(chosen.power, hero.defense);
            hero.health -= dmg;
            enemy.mp -= chosen.cost;
            spellSfx->play();

            message = isCrit
                ? QString("Critical spell! The dragon casts %1 and deals %2 damage!").arg(QString::fromStdString(chosen.name)).arg(dmg)
                : QString("The dragon casts %1 and deals %2 damage!").arg(QString::fromStdString(chosen.name)).arg(dmg);
            usedSpell = true;
        }
    }
    if (!usedSpell) {
        auto [dmg, isCrit] = calculateAttackDamage(enemy.offense, hero.defense);
        hero.health -= dmg;
        isCrit ? critSfx->play() : attackSfx->play();
        message = isCrit
            ? QString("Critical hit! The dragon deals %1 damage!").arg(dmg)
            : QString("The dragon attacks and deals %1 damage!").arg(dmg);
    }

    showNarrationOnly(message, [this]() {
        if (hero.health <= 0) {
            status = BATTLE_STATUS::LOSE;
            hero.health = 0;
            battleOver();
        } else {
            playerTurn = true;
            showMainMenu();
        }
        updateUI();
    });
}

void MainWindow::battleOver() {
    player->stop();
    player->setLoops(1);
    if (status == BATTLE_STATUS::WIN) {
        ui->dragon->hide();
        printTextScroll("The dragon was defeated!");
        player->setSource(QUrl("qrc:/assets/music/victory.wav"));
    } else {
        printTextScroll("You were defeated...");
        player->setSource(QUrl("qrc:/assets/music/lose.wav"));
    }
    player->play();

    QTimer::singleShot(6000, this, []() {
        QApplication::quit();
    });
}

static std::mt19937 rng(std::random_device{}());

float  MainWindow::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

std::pair<int, bool> MainWindow::calculateAttackDamage(int attackerOffense, int attackedDefense) {
    if (attackedDefense <= 0) attackedDefense = 1;
    float ratio = static_cast<float>(attackerOffense) / attackedDefense;
    float baseDamage = 10.0f * std::pow(ratio, 1.2f);
    baseDamage *= randomFloat(0.9f, 1.1f);

    bool isCritical = (randomFloat(0.f, 1.f) < 0.15f);
    if (isCritical) baseDamage *= 2.f;

    return {static_cast<int>(baseDamage), isCritical};
}

void MainWindow::playMusic(const QString& resourcePath) {
    player->stop();
    player->setSource(QUrl(resourcePath));
    player->setLoops(QMediaPlayer::Infinite);
    player->play();
}