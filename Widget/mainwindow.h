#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QMenuBar>
#include <QMessageBox>

#include <QProcess>
#include <QHBoxLayout>
#include <QSettings>
//#include <QDebug>

#include <Lib/header.h>

#include "Widget/sidebar.h"

#include "mw/HomePage/homepage.h"
#include "mw/Edit/editview.h"

#include "Dialog/settingsdialog.h"
#include "Dialog/newprojdialog.h"

#include "Menu/rflmenu.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
protected:
    void changeEvent(QEvent *ev) override;
    void closeEvent(QCloseEvent *ev) override;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setCurrentView(const QString &name);
    void setCurrentView(MainWindowView *view);

    void updateProjMenuState();

    void updateTr();

private slots:
    void onNewProj();
    void onOpenProj();
    void onOpenProj(const QString &filePath);
    void onSaveProj();
    void onSaveProjAs();

    void onAbout();

    void onRFLMenuRequested(const QPoint &pos);

private:
    QRect mNormalGeometry;   //默认位置大小

    QStackedWidget *mStackedWidget = new QStackedWidget;     //视图控件
    SideBar *mSideBar = new SideBar; //侧边栏


    struct Menu {
        QMenu file;
        QAction file_actNewProj;
        QAction file_actOpenProj;
        QAction file_actSave;
        QAction file_actSaveAs;

        QMenu other;
        QAction other_actAbout;
        QAction other_actAboutQt;
        QAction other_actSettings;

        void init(QMenuBar *menuBar);
        void tr();
    } mMenu;


    struct View
    {
        QString iconPath;
        QString name;
        QString(*TrFn)();
        MainWindowView *p;
    };

    HomePage *mViewHomePage = new HomePage;
    EditView *mViewEdit = new EditView;

    View mViews[2] = {
        { ":/SideBtn/qrc/HomePage.png", "HomePage", []()->QString { return tr("HomePage"); }, mViewHomePage },
        { ":/SideBtn/qrc/Edit.png", "Edit", []()->QString { return tr("Edit"); }, mViewEdit }
    };

    QMap<QString, MainWindowView*> mMapViews = {
        { "HomePage", mViewHomePage },
        { "Edit", mViewEdit }
    };
};
