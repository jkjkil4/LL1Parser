#-------------------------------------------------
#
# Project created by QtCreator 2020-12-06T18:15:30
#
#-------------------------------------------------

QT       += core gui qml

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

CONFIG += c++11 console

SOURCES += \
        Class/Parser/parser.cpp \
        Class/Parser/js.cpp \
        Class/Parser/jsobject.cpp \
        Class/ProjListWidgetItem/projlistwidgetitem.cpp \
        Class/texthighlighter.cpp \
        Widget/Dialog/newprojdialog.cpp \
        Widget/Dialog/settingsdialog.cpp \
        Widget/Dialog/delayedverifydialog.cpp \
        Widget/Dialog/checkboxverifydialog.cpp \
        Widget/Menu/rflmenu.cpp \
        Widget/Other/infowidget.cpp \
        Widget/mw/Edit/editview.cpp \
        Widget/mw/Edit/notewidget.cpp \
        Widget/mw/Edit/outputwidget.cpp \
        Widget/mw/Edit/plaintextedit.cpp \
        Widget/mw/Edit/projwidget.cpp \
        Widget/mw/HomePage/homepage.cpp \
        Widget/mw/HomePage/recentfilelistwidget.cpp \
        Widget/sidebar.cpp \
        main.cpp \
        Widget/mainwindow.cpp

HEADERS += \
        Class/Parser/parser.h \
        Class/Parser/js.h \
        Class/Parser/jsobject.h \
        Class/ProjListWidgetItem/projlistwidgetitem.h \
        Class/texthighlighter.h \
        Widget/Menu/rflmenu.h \
        Widget/Other/colorwidget.h \
        Class/translator.h \
        Widget/Dialog/newprojdialog.h \
        Widget/Dialog/settingsdialog.h \
        Widget/Dialog/delayedverifydialog.h \
        Widget/Dialog/checkboxverifydialog.h \
        Widget/Other/infowidget.h \
        Widget/mw/Edit/editview.h \
        Widget/mw/Edit/notewidget.h \
        Widget/mw/Edit/outputwidget.h \
        Widget/mw/Edit/plaintextedit.h \
        Widget/mw/Edit/projwidget.h \
        Widget/mw/HomePage/homepage.h \
        Widget/mw/HomePage/recentfilelistwidget.h \
        Widget/mw/mainwindowview.h \
        Widget/sidebar.h \
        Widget/mainwindow.h \
        header.h

DISTFILES += \
    tr/tr_zh_CN.ts

include(inc.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qrc.qrc
