#include "projwidget.h"

ProjWidget::ProjWidget(const CanonicalFilePath &projPath, QWidget *parent)
    : PlainTextEdit(parent), mProjName(QFileInfo(projPath).completeBaseName()), mProjPath(projPath)
{
    j::SetFamily(this, fontSourceCodePro.mFamily);
    j::SetPointSize(this, 10);
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::ClickFocus);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // j::SetFamily(mEdit, fontSourceCodePro.mFamily);
    // j::SetPointSize(mEdit, 10);

    // QLabel *labPath = new QLabel(projPath);
    // labPath->setAlignment(Qt::AlignLeft);
    // j::SetPointSize(labPath, 8);

    // ColorWidget *bottomWidget = new ColorWidget;
    // QWidget *mainWidget = new QWidget;

    // connect(mEdit, &PlainTextEdit::textChanged, [this]{ setSaved(false); });
    // connect(mEdit, &PlainTextEdit::pointSizeChanged, [this](int cur){
    //     mNoteWidget->setText(tr("Pointsize changed: %1 (Default: %2)").arg(QString::number(cur), "10"));
    //     mNoteWidget->setColor(qRgb(0, 0, 255));
    // });
    // connect(mOutputWidget->errListWidget(), SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onListWidgetDoubleClicked(QListWidgetItem*)));
    // connect(mOutputWidget->outputListWidget(), SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onListWidgetDoubleClicked(QListWidgetItem*)));
    // connect(mBtnParse, SIGNAL(clicked()), this, SLOT(onParse()));


    //创建布局
    // QHBoxLayout *layBottom = new QHBoxLayout;
    // layBottom->addWidget(mNoteWidget);
    // layBottom->addWidget(mBtnParse, 0, Qt::AlignRight);
    // bottomWidget->setLayout(layBottom);

    // QVBoxLayout *layCentral = new QVBoxLayout;
    // layCentral->setMargin(0);
    // layCentral->setSpacing(0);
    // layCentral->addSpacing(4);
    // layCentral->addWidget(labPath);
    // layCentral->addSpacing(4);
    // layCentral->addWidget(mEdit);
    // layCentral->addWidget(bottomWidget);
    // mainWidget->setLayout(layCentral);

    // mSplitter->addWidget(mainWidget);
    // mSplitter->addWidget(mOutputWidget);
    // mSplitter->setStretchFactor(0, 40);
    // mSplitter->setStretchFactor(1, 1);
    // mSplitter->setChildrenCollapsible(false);

    // QHBoxLayout *layMain = new QHBoxLayout;
    // layMain->setMargin(0);
    // layMain->addWidget(mSplitter);
    // setLayout(layMain);

    // updateTr();
}

bool ProjWidget::load() {
    QFile file(mProjPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    blockSignals(true);
    setPlainText(in.readAll());
    blockSignals(false);
    file.close();

    return true;
}

bool ProjWidget::save() {
    QFile file(mProjPath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << toPlainText();
    file.close();

    setSaved(true);

    return true;
}

void ProjWidget::setSaved(bool _isSaved) {
    if(_isSaved != mSaved) {
        mSaved = _isSaved;
        emit stateChanged(_isSaved);
    }
}

// void ProjWidget::updateTr() {
//     mBtnParse->setText(tr("Parse"));
// }

// void ProjWidget::onListWidgetDoubleClicked(QListWidgetItem *item) {
//     ProjListWidgetItem *uItem = (ProjListWidgetItem*)item;
//     emit processItemDbClick(uItem);
// }

// void ProjWidget::onParse() {
//     bool cancel = false;
//     emit beforeParse(cancel);
//     if(cancel) return;

//     mBtnParse->setEnabled(false);

//     mOutputWidget->clear();     //清除原有信息
//     QListWidget *errListWidget = mOutputWidget->errListWidget();
//     QListWidget *outputListWidget = mOutputWidget->outputListWidget();

//     QTime t;    //用于计时（包括分析时间和输出文件时间）
//     t.start();

//     Parser parser(mProjPath, this);
//     Parser::Result &result = parser.result();
//     if(!result.issues().hasError()) {   //如果没有错误，则显示输出文件列表项
//         QStringList paths = result.output();
//         if(!paths.isEmpty()) {
//             auto item = NewPlwiFn([paths] {
//                 QListWidget *listWidget = new QListWidget;
                
//                 //遍历所有输出文件路径，向listWidget添加item
//                 for(const QString &filePath : qAsConst(paths)) {
//                     QListWidgetItem *item = new QListWidgetItem(filePath);
//                     item->setIcon(QFileIconProvider().icon(filePath));
//                     listWidget->addItem(item);
//                 }
//                 //绑定itemDoubleClicked，以打开相应文件夹
//                 connect(listWidget, &QListWidget::itemDoubleClicked, [](QListWidgetItem *item) {
//                     QString path = QFileInfo(item->text()).path();
//                     if(QDir().exists(path))
//                         QDesktopServices::openUrl(QUrl("file:///" + path));
//                 });

//                 //总布局
//                 QLabel *label = new QLabel(tr("Double click to show in folder"));
//                 QVBoxLayout *layout = new QVBoxLayout;
//                 layout->addWidget(label, 0, Qt::AlignLeft);
//                 layout->addWidget(listWidget, 1);

//                 //显示窗口
//                 QWidget *widget = new QWidget;
//                 widget->setLayout(layout);
//                 widget->setAttribute(Qt::WA_DeleteOnClose);
//                 widget->setWindowTitle(tr("Files"));
//                 widget->resize(600, 400);
//                 widget->setMinimumSize(300, 200);
//                 widget->show();
//             });
//             item->setText(tr("Files have been outputed"));
//             item->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
//             outputListWidget->addItem(item);
//         }
//     }

//     int ms = t.elapsed();
//     ProjListWidgetItem *item = new ProjListWidgetItem(tr("Elapsed time: %1ms").arg(ms));
//     item->setForeground(Qt::blue);
//     outputListWidget->insertItem(0, item);

//     mOutputWidget->setCurrentWidget(result.issues().hasError() ? errListWidget : outputListWidget);

//     QString strRow = tr("Row:%1");
//     QString strCol = tr("Col:%1");
//     for(const Parser::Issue &issue : result.issues().list()) {  //遍历错误并显示到错误列表中
//         //得到文本
//         QString text;
//         if(!issue.filePath.isEmpty())
//             text += QFileInfo(issue.filePath).fileName() + ' ';
//         if(issue.row != -1) {
//             text += strRow.arg(issue.row + 1) + ' ';
//             if(issue.col != -1)
//                 text += strCol.arg(issue.col + 1) + ' ';
//         }

//         //添加
//         ProjListWidgetItem *item = issue.newItem();
//         if(!item)
//             item = new ProjListWidgetItem;
//         item->setIcon(issue.icon());
//         item->setText(text.isEmpty() ? issue.what : "[ " + text + "] " + issue.what);
//         errListWidget->addItem(item);
//     }

//     QList<Parser::JSDebugMessage> jsDebugMsg = result.jsDebugMessage();
//     if(!jsDebugMsg.isEmpty()) {     //如果有调试信息，则显示调试信息项
//         Parser::ValueMap<CanonicalFilePath> files = result.files();
//         auto item = NewPlwiFn([files, jsDebugMsg]{
//             //lambda 用于显示调试信息
//             auto fnShowMsg = [](const QString &fileName, const QString &text) {
//                 QPlainTextEdit *edit = new QPlainTextEdit;
//                 edit->setAttribute(Qt::WA_DeleteOnClose);
//                 edit->setWindowTitle(tr("Debug Message of \"%1\"").arg(fileName));
//                 edit->setPlainText(text);
//                 edit->resize(400, 340);
//                 j::SetFamily(edit, fontSourceCodePro.mFamily);
//                 j::SetPointSize(edit, 10);
//                 edit->show();
//             };

//             //如果只有一个文件有调试信息，则直接显示，否则弹出列表进行选择
//             if(jsDebugMsg.size() == 1) {
//                 const Parser::JSDebugMessage &msg = jsDebugMsg[0];
//                 fnShowMsg(QFileInfo(files.indexKey(msg.fileId)).fileName(), msg.text);
//             } else {
//                 QListWidget *listWidget = new QListWidget;

//                 //遍历所有调试信息，向listWidget添加item
//                 for(const Parser::JSDebugMessage &msg : qAsConst(jsDebugMsg)) {
//                     QListWidgetItem *item = new QListWidgetItem(files.indexKey(msg.fileId));
//                     item->setData(Qt::UserRole, msg.text);
//                     listWidget->addItem(item);
//                 }
//                 //绑定itemDoubleClicked，以显示对应的调试信息
//                 connect(listWidget, &QListWidget::itemDoubleClicked, [fnShowMsg](QListWidgetItem *item) {
//                     fnShowMsg(QFileInfo(item->text()).fileName(), item->data(Qt::UserRole).toString());
//                 });

//                 //总布局
//                 QLabel *label = new QLabel(tr("Double click to show detail"));
//                 QVBoxLayout *layout = new QVBoxLayout;
//                 layout->addWidget(label, 0, Qt::AlignLeft);
//                 layout->addWidget(listWidget, 1);

//                 //显示窗口
//                 QWidget *widget = new QWidget;
//                 widget->setLayout(layout);
//                 widget->setAttribute(Qt::WA_DeleteOnClose);
//                 widget->setWindowTitle(tr("Debug Message"));
//                 widget->resize(600, 400);
//                 widget->setMinimumSize(300, 200);
//                 widget->show();
//             }
//         });
//         item->setText(tr("JS debug message"));
//         item->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
//         outputListWidget->addItem(item);
//     }

//     //如果没有错误，则显示相关信息
//     if(!result.issues().hasError()) {
//         //文件
//         QString trFiles = tr("Analysised files");
//         PLWI_ShowPlainText *itemFiles = new PLWI_ShowPlainText(trFiles, result.formatFiles());
//         itemFiles->setText(trFiles);
//         itemFiles->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
//         outputListWidget->addItem(itemFiles);

//         //空串情况
//         QString trSymbolsNil = tr("Empty string state");
//         PLWI_ShowPlainText *itemSymbolsNil = new PLWI_ShowPlainText(trSymbolsNil, result.formatSymbolsNil());
//         itemSymbolsNil->setText(trSymbolsNil);
//         itemSymbolsNil->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
//         outputListWidget->addItem(itemSymbolsNil);

//         //FIRST集
//         QString trFirstSet = tr("FIRST set");
//         PLWI_ShowHtmlText *itemFirstSet = new PLWI_ShowHtmlText(trFirstSet, result.formatFirstSet(true));
//         itemFirstSet->setText(trFirstSet);
//         itemFirstSet->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
//         outputListWidget->addItem(itemFirstSet);

//         //FOLLOW集
//         QString trFollowSet = tr("FOLLOW set");
//         PLWI_ShowHtmlText *itemFollowSet = new PLWI_ShowHtmlText(trFollowSet, result.formatFollowSet(true));
//         itemFollowSet->setText(trFollowSet);
//         itemFollowSet->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
//         outputListWidget->addItem(itemFollowSet);

//         //SELECT集
//         QString trSelectSet = tr("SELECT set");
//         PLWI_ShowHtmlText *itemSelectSet = new PLWI_ShowHtmlText(trSelectSet, result.formatSelectSet(true));
//         itemSelectSet->setText(trSelectSet);
//         itemSelectSet->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
//         outputListWidget->addItem(itemSelectSet);
//     }

//     mNoteWidget->setText(tr("Analysis completed"));
//     mNoteWidget->setColor(qRgb(0, 128, 0));
//     mBtnParse->setEnabled(true);
// }

// void ProjWidget::changeEvent(QEvent *ev) {
//     if(ev->type() == QEvent::LanguageChange) {
//         updateTr();
//     }
// }

void ProjWidget::focusInEvent(QFocusEvent *ev) {
    PlainTextEdit::focusInEvent(ev);
    if(QRect(0, 0, width(), height()).contains(mapFromGlobal(cursor().pos())))
        emit focused();
}
