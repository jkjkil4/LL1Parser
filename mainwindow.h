#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

#include <QHBoxLayout>
#include <QSettings>
#include <QDebug>

#include <header.h>

#include "Widget/mw/mainwindowview.h"
#include "Widget/mw/welcomeview.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    bool setCurrentView(const QString &viewName);   //设置当前视图

private:
    QRect normalGeometry;   //默认位置大小

    QStackedWidget *stackedWidget = new QStackedWidget;     //视图控件

    QMap<QString, MainWindowView*> mapWidgets = {
        { "Welcome", new WelcomeView }
    };
};

#endif // MAINWINDOW_H
