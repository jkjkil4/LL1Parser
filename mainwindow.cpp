#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //创建控件
    for(auto widget : mapWidgets) { //遍历所有的视图
        stackedWidget->addWidget(widget);   //将该视图添加至stackedWidget中
        connect(widget, &MainWindowView::changeView, [=](const QString &viewName){ setCurrentView(viewName); });    //绑定信号与槽
    }
    setCurrentView("Welcome");  //设置当前视图为"Welcome"


    //创建布局
    QHBoxLayout *layMain = new QHBoxLayout; //主布局
    layMain->setMargin(0);
    layMain->setSpacing(0);
    layMain->addWidget(stackedWidget);

    QWidget *centralWidget = new QWidget;   //中心控件
    centralWidget->setLayout(layMain);
    setCentralWidget(centralWidget);


    //设置窗口位置大小
    resize(800, 608);
    setMinimumSize(500, 400);
    adjustSize();

    QSettings config(APP_DIR + "/config/config.ini", QSettings::IniFormat);     //读取ini

    //读取窗口位置大小
    normalGeometry = config.value("MW/Geometry", geometry()).toRect();
    setGeometry(normalGeometry);
    if(config.value("MW/IsMaximized", true).toBool())
        showMaximized();
}

MainWindow::~MainWindow()
{
    QSettings config(APP_DIR + "/config/config.ini", QSettings::IniFormat);
    config.setValue("MW/Geometry", (isMaximized() || isMinimized()) ? normalGeometry : geometry());
    config.setValue("MW/IsMaximized", isMaximized());
}

bool MainWindow::setCurrentView(const QString &viewName) {
    auto iter = mapWidgets.find(viewName);
    if(iter == mapWidgets.end())
        return false;
    stackedWidget->setCurrentWidget(*iter);
    return true;
}

