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
        sideBar->append(QIcon(view.iconPath), view.p, view.TrFn());
        connect(view.p, &MainWindowView::changeView, [this](const QString &viewName){ setCurrentView(viewName); });    //绑定信号与槽
    }
    setCurrentView(viewHomePage);  //设置当前视图为"HomePage"

    connect(sideBar, &SideBar::actived, [this](const SideBar::Data &data){ setCurrentView(data.view); });
    connect(viewHomePage->recentFileListWidget(), &RFLWidget::itemClicked, [this](const RFLWidget::Item &item){ onOpenProj(item.filePath); });
    connect(viewHomePage->recentFileListWidget(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onRFLMenuRequested(const QPoint&)));
    connect(viewHomePage->btnNew(), SIGNAL(clicked()), this, SLOT(onNewProj()));
    connect(viewHomePage->btnOpen(), SIGNAL(clicked()), this, SLOT(onOpenProj()));

    //菜单
    menu.init(menuBar());
    connect(&menu.file_actNewProj, SIGNAL(triggered(bool)), this, SLOT(onNewProj()));
    connect(&menu.file_actOpenProj, SIGNAL(triggered(bool)), this, SLOT(onOpenProj()));
    connect(&menu.other_actAbout, SIGNAL(triggered(bool)), this, SLOT(onAbout()));
    connect(&menu.other_actAboutQt, &QAction::triggered, [this]{ QMessageBox::aboutQt(this); });
    connect(&menu.other_actSettings, &QAction::triggered, [this]{ SettingsDialog(this).exec(); });


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
    //保存设定
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
    NewProjDialog dialog(this);		//对话框
    if(dialog.exec()) {		//若选择了"Ok"
        //得到路径
        QString path = dialog.projPath();
        QString name = dialog.projName();
        name += "." SUFFIX;
        QDir dir(path);
        QString filePath = dir.absoluteFilePath(name);

        //若文件存在，询问是否覆盖
        int res = dir.exists(name)
                ? QMessageBox::information(this, "", tr("Are you sure to overlay \"%1\"?").arg(filePath), QMessageBox::Ok, QMessageBox::Cancel)
                : QMessageBox::Ok;

        if(res != QMessageBox::Ok)	//若不覆盖，则return
            return;

        //写入空文件
	    QFile file(filePath);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Cannot create the project"));
            return;
        }
        file.close();
        onOpenProj(filePath);
    }
}

void MainWindow::onOpenProj() {
    QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Project"), config.value("Path/OpenProj").toString(), "LL1Parser Project (*.lpp)");
    if(filePath.isEmpty())
        return;
    config.setValue("Path/OpenProj", QFileInfo(filePath).path());
    onOpenProj(filePath);
}

void MainWindow::onOpenProj(const QString &filePath) {
    rfManager.append(filePath);

    //TODO: 打开项目
    sideBar->setCurrent(viewEdit);
}

void MainWindow::onAbout() {
    QString text;
    QTextStream ts(&text);
    ts << tr("Author") << ": jkjkil4<br>"
       << "github: <a href=https://github.com/jkjkil4/LL1Parser>https://github.com/jkjkil4/LL1Parser</a><br>"
       << tr("Feedback") << ": jkjkil@qq.com" ;
    QMessageBox::about(this, QApplication::tr("About"), text);
}
#include <QDebug>
void MainWindow::onRFLMenuRequested(const QPoint &pos) {
    RFLWidget::Item item = viewHomePage->recentFileListWidget()->itemAt(pos.y());
    if(item.row == -1)
        return;

    RFLMenu menu;
    menu.move(cursor().pos());
    QAction *res = menu.exec();

    if(res == menu.actMoveToFirst) {
        rfManager.append(item.filePath);
    } else if(res == menu.actRemove) {
        rfManager.remove(item.row);
    } else if(res == menu.actShowInExplorer) {
#ifdef Q_OS_WIN
        QProcess::startDetached("cmd.exe", QStringList() << "/c" << "start" << "" << QFileInfo(item.filePath).path());
#else
        QMessageBox::information(this, "", tr("This function is not supported in this operating system"));
#endif
    }
}

void MainWindow::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}

