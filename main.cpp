#include "Widget/mainwindow.h"
#include <QtSingleApplication>
#include <QDir>

#include "Class/translator.h"

#include <QDebug>
#include "Widget/mw/Edit/tabsplitwidget.h"

Translator translator;
RecentFileManager rfManager;

FontFamily fontSourceCodePro;

int main(int argc, char *argv[])
{
    QtSingleApplication a(argc, argv);

    QStringList args = a.arguments();
    bool isOpen = args.length() == 2 && QDir().exists(args[1]);
    if(a.isRunning()) {
        a.sendMessage(isOpen ? args[1] : "", 2000);
        return 0;
    } else {
        QDir appDir(APP_DIR);
        appDir.mkdir("Config");
        //appDir.mkdir("Languages");

        translator.setApplication(&a);
        translator.loadLocale();
        rfManager.setFilePath(APP_DIR + "/Config/rfl.txt");
        fontSourceCodePro.load(":/qrc/SourceCodePro-Medium.ttf");

        MainWindow w;
        a.setActivationWindow(&w);
        QObject::connect(&a, &QtSingleApplication::messageReceived, [&a, &w](const QString &msg) {
            if(!msg.isEmpty())
                w.onOpenProj(msg);
            a.activateWindow();
        });
        if(isOpen)
            w.onOpenProj(args[1]);
        w.show();  

        int res = a.exec();

        translator.saveLocale();
        rfManager.save();
        return res;
    }
}
