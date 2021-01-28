#-------------------------------------------------
#
# Project created by QtCreator 2020-12-06T18:15:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LL1Parser
TEMPLATE = app

RC_ICONS += LL1Parser.ico

TRANSLATIONS += tr/tr_zh_CN.ts

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        Widget/Dialog/newprojdialog.cpp \
        Widget/Other/infowidget.cpp \
        Widget/mw/HomePage/homepage.cpp \
        Widget/mw/HomePage/recentfilelistwidget.cpp \
        Widget/Dialog/settingsdialog.cpp \
        Widget/sidebar.cpp \
        main.cpp \
        Widget/mainwindow.cpp

HEADERS += \
        Widget/Other/colorwidget.h \
        Class/translator.h \
        Widget/Dialog/newprojdialog.h \
        Widget/Other/infowidget.h \
        Widget/mw/HomePage/homepage.h \
        Widget/mw/HomePage/recentfilelistwidget.h \
        Widget/mw/mainwindowview.h \
        Widget/Dialog/settingsdialog.h \
        Widget/sidebar.h \
        Widget/mainwindow.h \
        header.h

DISTFILES += \
    tr/tr_zh_CN.ts

include($(Lib)/MyQtLibs/myqtlibs.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qrc.qrc
