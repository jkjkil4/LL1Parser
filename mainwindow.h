#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

#include <QHBoxLayout>
#include <QSettings>
//#include <QDebug>

#include <Lib/header.h>

#include "Widget/sidebar.h"

#include "Widget/mw/mainwindowview.h"
#include "Widget/mw/HomePage/homepage.h"

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

private:
    QRect normalGeometry;   //默认位置大小

    QStackedWidget *stackedWidget = new QStackedWidget;     //视图控件
    SideBar *sideBar = new SideBar; //侧边栏

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

#endif // MAINWINDOW_H
