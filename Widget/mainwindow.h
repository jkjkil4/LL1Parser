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

#include "Dialog/settingsdialog.h"
#include "Dialog/newprojdialog.h"

#include "Menu/rflmenu.h"

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
    void onNewProj();
    void onOpenProj();
    void onOpenProj(const QString &filePath);

    void onAbout();

    void onRFLMenuRequested();

private:
    QRect normalGeometry;   //默认位置大小

    QStackedWidget *stackedWidget = new QStackedWidget;     //视图控件
    SideBar *sideBar = new SideBar; //侧边栏


    struct Menu {
        QMenu file;
        QAction file_actNewProj;
        QAction file_actOpenProj;

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
        QString name;
        QString(*TrFn)();
        MainWindowView *p;
    };

    HomePage *viewHomePage = new HomePage;
    MainWindowView *tmp = new MainWindowView;

    View views[2] = {
        { ":/SideBtn/qrc/HomePage.png", "HomePage", []()->QString { return tr("HomePage"); }, viewHomePage },
        { ":/SideBtn/qrc/Edit.png", "Edit", []()->QString { return tr("Edit"); }, tmp }
    };

    QMap<QString, MainWindowView*> mapViews = {
        { "HomePage", viewHomePage },
        { "Edit", tmp }
    };
};
