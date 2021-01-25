#include "Widget/mainwindow.h"
#include <QApplication>
#include <QDir>

#include "Class/translator.h"

Translator translator;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    translator.setApplication(&a);
    translator.loadLocale();

    QDir dir(APP_DIR);
    dir.mkdir("Config");
    dir.mkdir("Languages");

    MainWindow w;
    w.show();

    translator.saveLocale();

    int res = a.exec();
    translator.saveLocale();
    return res;
}
