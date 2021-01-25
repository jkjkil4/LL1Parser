#pragma once

#include <QTranslator>
#include <QApplication>
#include <QSettings>

#include <Lib/header.h>

class Translator
{
public:
    void setApplication(QApplication *app) {
        app->installTranslator(&trans);
        app->installTranslator(&transQt);
    }
    bool setLocale(const QString& _locale) {
        locale = _locale;
        QString use = (locale == "Auto" ? QLocale::system().name() : locale);
        return trans.load("tr_" + use, APP_DIR + "/Languages") && transQt.load("qt_" + use, APP_DIR + "/Languages/Qt");
    }
    bool loadLocale() {
        QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
        return setLocale(config.value("Config/TrLocale", "Auto").toString());
    }
    void saveLocale() {
        QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
        config.setValue("Config/TrLocale", locale);
    }
    VAR_GET_FUNC(Locale, locale, QString)

private:
    QString locale;

    QTranslator trans;
    QTranslator transQt;
};

extern Translator translator;
