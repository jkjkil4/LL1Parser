#include "mainwindow.h"
#include <QApplication>

#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDir dir(APP_DIR);
    dir.mkdir("Config");

    MainWindow w;
    w.show();

    return a.exec();
}
