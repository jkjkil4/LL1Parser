#include "mainwindow.h"
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //创建控件
    for(auto &view : views) { //遍历所有的视图
        stackedWidget->addWidget(view.p);   //将该视图添加至stackedWidget中
        sideBar->append(QIcon(view.iconPath), view.TrFn());
        connect(view.p, &MainWindowView::changeView, [this](const QString &viewName){ setCurrentView(viewName); });    //绑定信号与槽
    }
    setCurrentView(viewHomePage);  //设置当前视图为"HomePage"

    j::LimitWidth(sideBar, 70);
    connect(sideBar, &SideBar::clicked, this, [this](const SideBar::Data &data){ setCurrentView(data.text); });


    //创建布局
    QHBoxLayout *layMain = new QHBoxLayout; //主布局
    layMain->setMargin(0);
    layMain->setSpacing(0);
    layMain->addWidget(sideBar);
    layMain->addWidget(stackedWidget);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(layMain);
    setCentralWidget(centralWidget);


    //设置窗口位置大小
    resize(800, 608);
    //setMinimumSize(500, 400);
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

void MainWindow::setCurrentView(const QString &name) {
    MainWindowView *view = mapViews.value(name, nullptr);
    if(view) setCurrentView(view);
}
void MainWindow::setCurrentView(MainWindowView *view) {
    stackedWidget->setCurrentWidget(view);
}

void MainWindow::updateTr() {
    int index = 0;
    for(View &view : views) {
        sideBar->setText(index, view.TrFn());
        index++;
    }
}

void MainWindow::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}

