#include "Widget/mainwindow.h"
#include <QApplication>
#include <QDir>

#include "Class/translator.h"

Translator translator;
RecentFileManager rfManager;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDir appDir(APP_DIR);
    appDir.mkdir("Config");
    appDir.mkdir("Languages");

    translator.setApplication(&a);
    translator.loadLocale();

    rfManager.setFilePath(APP_DIR + "/Config/rfl.txt");

    MainWindow w;
    w.show();

    translator.saveLocale();

    int res = a.exec();
    translator.saveLocale();
    return res;
}
