#include "mainwindow.h"
#include <QCoreApplication>
#include <QApplication>

void MainWindow::Menu::init(QMenuBar *menuBar) {
    file_actNewProj.setShortcut(QString("Ctrl+N"));
    file_actOpenProj.setShortcut(QString("Ctrl+O"));
    file_actSave.setShortcut(QString("Ctrl+S"));
    //file_actSaveAs

    //other_actAbout
    //other_actAboutQt
    //other_actSettings


    menuBar->addMenu(&file);
    file.addAction(&file_actNewProj);
    file.addAction(&file_actOpenProj);
    file.addSeparator();
    file.addAction(&file_actSave);
    file.addAction(&file_actSaveAs);

    menuBar->addMenu(&other);
    other.addAction(&other_actAbout);
    other.addAction(&other_actAboutQt);
    other.addSeparator();
    other.addAction(&other_actSettings);
}

void MainWindow::Menu::tr() {
    file.setTitle(QApplication::tr("File") + "(&F)");
    file_actNewProj.setText(QApplication::tr("New Project") + "(&N)");
    file_actOpenProj.setText(QApplication::tr("Open Project") + "(&O)");
    file_actSave.setText(QApplication::tr("Save Project") + "(&S)");
    file_actSaveAs.setText(QApplication::tr("Save Project As") + "(&A)");

    other.setTitle(QApplication::tr("Other") + "(&O)");
    other_actAbout.setText(QApplication::tr("About") + "(&A)");
    other_actAboutQt.setText(QApplication::tr("AboutQt") + "(&Q)");
    other_actSettings.setText(QApplication::tr("Settings") + "(&S)");
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //创建控件
    for(auto &view : mViews) { //遍历所有的视图
        mStackedWidget->addWidget(view.p);   //将该视图添加至stackedWidget中
        mSideBar->append(QIcon(view.iconPath), view.p, view.TrFn());
        connect(view.p, &MainWindowView::changeView, [this](const QString &viewName){ setCurrentView(viewName); });    //绑定信号与槽
    }
    setCurrentView(mViewHomePage);  //设置当前视图为"HomePage"

    connect(mSideBar, &SideBar::actived, [this](const SideBar::Data &data){
        mStackedWidget->setCurrentWidget(data.view);
        updateProjMenuState();
    });
    connect(mViewHomePage->recentFileListWidget(), &RFLWidget::itemClicked, [this](const RFLWidget::Item &item){ onOpenProj(item.filePath); });
    connect(mViewHomePage->recentFileListWidget(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onRFLMenuRequested(const QPoint&)));
    connect(mViewHomePage->btnNew(), SIGNAL(clicked()), this, SLOT(onNewProj()));
    connect(mViewHomePage->btnOpen(), SIGNAL(clicked()), this, SLOT(onOpenProj()));

    //菜单
    mMenu.init(menuBar());
    connect(&mMenu.file_actNewProj, SIGNAL(triggered(bool)), this, SLOT(onNewProj()));
    connect(&mMenu.file_actOpenProj, SIGNAL(triggered(bool)), this, SLOT(onOpenProj()));
    connect(&mMenu.file_actSave, SIGNAL(triggered(bool)), this, SLOT(onSaveProj()));
    connect(&mMenu.file_actSaveAs, SIGNAL(triggered(bool)), this, SLOT(onSaveProjAs()));
    connect(&mMenu.other_actAbout, SIGNAL(triggered(bool)), this, SLOT(onAbout()));
    connect(&mMenu.other_actAboutQt, &QAction::triggered, [this]{ QMessageBox::aboutQt(this); });
    connect(&mMenu.other_actSettings, &QAction::triggered, [this]{ SettingsDialog(this).exec(); });


    //创建布局
    QHBoxLayout *layMain = new QHBoxLayout; //主布局
    layMain->setMargin(0);
    layMain->setSpacing(0);
    layMain->addWidget(mSideBar);
    layMain->addWidget(mStackedWidget);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(layMain);
    setCentralWidget(centralWidget);


    //设置窗口位置大小
    resize(800, 608);
    adjustSize();

    QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);     //读取ini

    //读取窗口位置大小
    mNormalGeometry = config.value("MW/Geometry", geometry()).toRect();
    setGeometry(mNormalGeometry);
    if(config.value("MW/IsMaximized", true).toBool())
        showMaximized();

    updateTr();
    updateProjMenuState();
}

MainWindow::~MainWindow()
{
    //保存设定
    QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
    config.setValue("MW/Geometry", (isMaximized() || isMinimized()) ? mNormalGeometry : geometry());
    config.setValue("MW/IsMaximized", isMaximized());
}

void MainWindow::setCurrentView(const QString &name) {
    MainWindowView *view = mMapViews.value(name, nullptr);
    if(view) setCurrentView(view);
}
void MainWindow::setCurrentView(MainWindowView *view) {
    mSideBar->setCurrent(view);
}

void MainWindow::updateProjMenuState() {
    bool atProj = mSideBar->current() == mViewEdit && mViewEdit->count() != 0;
    mMenu.file_actSave.setEnabled(atProj);
    mMenu.file_actSaveAs.setEnabled(atProj);
}

void MainWindow::updateTr() {
    int index = 0;
    for(View &view : mViews) {
        mSideBar->setText(index, view.TrFn());
        index++;
    }
    mMenu.tr();
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

    mViewEdit->open(filePath);
    mSideBar->setCurrent(mViewEdit);
}

void MainWindow::onSaveProj() {
    ProjWidget *proj = mViewEdit->current();
    if(proj && !proj->isSaved()) {
        if(!proj->save())
            QMessageBox::warning(this, tr("Error"), tr("Cannot save the project \"%1\"").arg(proj->projName()));
    }
}

void MainWindow::onSaveProjAs() {
    QMessageBox::information(this, "", tr("This function is not done"));
}

void MainWindow::onAbout() {
    QString text;
    QTextStream ts(&text);
    ts << tr("Author") << ": jkjkil4<br>"
       << "github: <a href=https://github.com/jkjkil4/LL1Parser>https://github.com/jkjkil4/LL1Parser</a><br>"
       << tr("Feedback") << ": jkjkil@qq.com" ;
    QMessageBox::about(this, QApplication::tr("About"), text);
}

void MainWindow::onRFLMenuRequested(const QPoint &pos) {
    RFLWidget::Item item = mViewHomePage->recentFileListWidget()->itemAt(pos.y());
    if(item.row == -1)
        return;

    RFLMenu menu;
    menu.move(cursor().pos());
    QAction *res = menu.exec();

    if(res == menu.mActMoveToFirst) {
        rfManager.append(item.filePath);
    } else if(res == menu.mActRemove) {
        rfManager.remove(item.row);
    } else if(res == menu.mActShowInExplorer) {
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
void MainWindow::closeEvent(QCloseEvent *ev) {
    if(!mViewEdit->closeAll())
        ev->ignore();
}

