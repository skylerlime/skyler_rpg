#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    int fontId = QFontDatabase::addApplicationFont(":/assets/dragon-quest.otf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.empty()) {
        QFont customFont(fontFamilies.at(0));
        a.setFont(customFont);
    } else {
        qWarning("Failed to load custom font.");
    }

    MainWindow w;
    w.show();
    return a.exec();
}
