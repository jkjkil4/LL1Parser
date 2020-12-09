#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDir dir(APP_DIR);
    dir.mkdir("config");

    MainWindow w;
    w.show();

    return a.exec();
}
