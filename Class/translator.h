#pragma once

#include <QTranslator>
#include <QApplication>
#include <QSettings>

#include <Lib/header.h>

class Translator : public QTranslator
{
public:
    void setApplication(QApplication *app) {
        app->installTranslator(this);
    }
    bool setLocale(const QString& _locale) {
        locale = _locale;
        return load("tr_" + (locale == "Auto" ? QLocale::system().name() : locale), APP_DIR + "/Languages");
    }
    bool loadLocale() {
        QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
        return setLocale(config.value("Config/TrLocale", QLocale::system().name()).toString());
    }
    void saveLocale() {
        QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
        config.setValue("Config/TrLocale", locale);
    }
    VAR_GET_FUNC(Locale, locale, QString)

private:
    QString locale;
};

extern Translator translator;
