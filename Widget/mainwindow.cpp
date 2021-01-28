#include "mainwindow.h"
#include <QCoreApplication>
#include <QApplication>

void MainWindow::Menu::init(QMenuBar *menuBar) {
    menuBar->addMenu(&file);
    file.addAction(&file_actNewProj);
    file.addAction(&file_actOpenProj);

    menuBar->addMenu(&other);
    other.addAction(&other_actAbout);
    other.addAction(&other_actAboutQt);
    other.addSeparator();
    other.addAction(&other_actSettings);
}

void MainWindow::Menu::tr() {
    file.setTitle(QApplication::tr("File"));
    file_actNewProj.setText(QApplication::tr("New Project"));
    file_actOpenProj.setText(QApplication::tr("Open Project"));

    other.setTitle(QApplication::tr("Other"));
    other_actAbout.setText(QApplication::tr("About"));
    other_actAboutQt.setText(QApplication::tr("AboutQt"));
    other_actSettings.setText(QApplication::tr("Settings"));
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //创建控件
    for(auto &view : views) { //遍历所有的视图
        stackedWidget->addWidget(view.p);   //将该视图添加至stackedWidget中
        sideBar->append(QIcon(view.iconPath), view.name, view.TrFn());
        connect(view.p, &MainWindowView::changeView, [this](const QString &viewName){ setCurrentView(viewName); });    //绑定信号与槽
    }
    setCurrentView(viewHomePage);  //设置当前视图为"HomePage"

    connect(sideBar, &SideBar::clicked, [this](const SideBar::Data &data){ setCurrentView(data.name); });
    connect(viewHomePage->recentFileListWidget(), SIGNAL(itemClicked(const QString&)), this, SLOT(onOpenProj(const QString&)));
    connect(viewHomePage->btnNew(), SIGNAL(clicked()), this, SLOT(onNewProj()));
    connect(viewHomePage->btnOpen(), SIGNAL(clicked()), this, SLOT(onOpenProj()));

    //菜单
    menu.init(menuBar());
    connect(&menu.file_actNewProj, SIGNAL(triggered(bool)), this, SLOT(onNewProj()));
    connect(&menu.file_actOpenProj, SIGNAL(triggered(bool)), this, SLOT(onOpenProj()));
    connect(&menu.other_actAbout, SIGNAL(triggered(bool)), this, SLOT(onAbout()));
    connect(&menu.other_actAboutQt, &QAction::triggered, [this]{ QMessageBox::aboutQt(this); });
    connect(&menu.other_actSettings, &QAction::triggered, []{ SettingsDialog().exec(); });


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
    adjustSize();

    QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);     //读取ini

    //读取窗口位置大小
    normalGeometry = config.value("MW/Geometry", geometry()).toRect();
    setGeometry(normalGeometry);
    if(config.value("MW/IsMaximized", true).toBool())
        showMaximized();

    updateTr();
}

MainWindow::~MainWindow()
{
    QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
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
    menu.tr();
}

void MainWindow::onNewProj() {
    NewProjDialog dialog(this);
    if(dialog.exec()) {
        QString path = dialog.projPath();
        QString name = dialog.projName();
        QDir dir(path);

    }
}

void MainWindow::onOpenProj() {
    QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Project"), config.value("Path/OpenProj").toString(), "*.ll1p");
    if(filePath.isEmpty())
        return;
    config.setValue("Path/OpenProj", QFileInfo(filePath).path());
    onOpenProj(filePath);
}

void MainWindow::onOpenProj(const QString &filePath) {
    Q_UNUSED(filePath)
    //TODO: 打开项目
}

void MainWindow::onAbout() {
    QString text;
    QTextStream ts(&text);
    ts << tr("Author") << ": jkjkil4<br>"
       << "github: <a href=https://github.com/jkjkil4/LL1Parser>https://github.com/jkjkil4/LL1Parser</a><br>"
       << tr("Feedback") << ": jkjkil@qq.com" ;
    QMessageBox::about(this, QApplication::tr("About"), text);
}

void MainWindow::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}

