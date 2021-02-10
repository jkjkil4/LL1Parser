#include "Widget/mainwindow.h"
#include <QApplication>
#include <QDir>

#include "Class/translator.h"

Translator translator;
RecentFileManager rfManager;

FontFamily fontSourceCodePro;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDir appDir(APP_DIR);
    appDir.mkdir("Config");
    //appDir.mkdir("Languages");

    translator.setApplication(&a);
    translator.loadLocale();
    rfManager.setFilePath(APP_DIR + "/Config/rfl.txt");
    fontSourceCodePro.load(":/qrc/SourceCodePro-Medium.ttf");

    MainWindow w;
    w.show();

    int res = a.exec();

    translator.saveLocale();
    rfManager.save();

    return res;
}
