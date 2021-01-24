#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QMenuBar>
#include <QMessageBox>

#include <QHBoxLayout>
#include <QSettings>
//#include <QDebug>

#include <Lib/header.h>

#include "Widget/sidebar.h"

#include "mw/mainwindowview.h"
#include "mw/HomePage/homepage.h"
#include "settingsdialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
protected:
    void changeEvent(QEvent *ev) override;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setCurrentView(const QString &name);
    void setCurrentView(MainWindowView *view);

    void updateTr();

private slots:
    void onAbout();

private:
    QRect normalGeometry;   //默认位置大小

    QStackedWidget *stackedWidget = new QStackedWidget;     //视图控件
    SideBar *sideBar = new SideBar; //侧边栏


    struct Menu {
        QMenu other;
        QAction other_actAbout;
        QAction other_actAboutQt;
        QAction other_actSettings;

        void init(QMenuBar *menuBar);
        void tr();
    } menu;


    struct View
    {
        QString iconPath;
        QString(*TrFn)();
        MainWindowView *p;
    };

    HomePage *viewHomePage = new HomePage;
    MainWindowView *tmp = new MainWindowView;

    View views[2] = {
        { ":/SideBtn/qrc/HomePage.png", []()->QString { return tr("HomePage"); }, viewHomePage },
        { ":/SideBtn/qrc/Edit.png", []()->QString { return tr("Edit"); }, tmp }
    };

    QMap<QString, MainWindowView*> mapViews = {
        { "HomePage", viewHomePage },
        { "Edit", tmp }
    };
};
