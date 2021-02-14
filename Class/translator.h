#pragma once

#include <QTranslator>
#include <QApplication>
#include <QSettings>

#include <Lib/header.h>

class Translator
{
public:
    void setApplication(QApplication *app) {
        app->installTranslator(&mTrans);
        app->installTranslator(&mTransQt);
    }
    bool setLocale(const QString& _locale) {
        mLocale = _locale;
        QString use = (mLocale == "Auto" ? QLocale::system().name() : mLocale);
        return mTrans.load("tr_" + use, APP_DIR + "/Languages") && mTransQt.load("qt_" + use, APP_DIR + "/Languages/Qt");
    }
    bool loadLocale() {
        QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
        return setLocale(config.value("Config/TrLocale", "Auto").toString());
    }
    void saveLocale() {
        QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
        config.setValue("Config/TrLocale", mLocale);
    }
    QString locale() { return mLocale; }

private:
    QString mLocale;

    QTranslator mTrans;
    QTranslator mTransQt;
};

extern Translator translator;
