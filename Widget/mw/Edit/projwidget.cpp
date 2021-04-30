#include "projwidget.h"

ProjWidget::ProjWidget(const QString &projPath, QWidget *parent)
    : QWidget(parent), mProjName(QFileInfo(projPath).completeBaseName()), mProjPath(projPath)
{
    j::SetFamily(mEdit, fontSourceCodePro.mFamily);
    j::SetPointSize(mEdit, 10);

    QLabel *labPath = new QLabel(projPath);
    labPath->setAlignment(Qt::AlignLeft);
    j::SetPointSize(labPath, 8);

    ColorWidget *bottomWidget = new ColorWidget;
    QWidget *mainWidget = new QWidget;

    connect(mEdit, &PlainTextEdit::textChanged, [this]{ setSaved(false); });
    connect(mEdit, &PlainTextEdit::pointSizeChanged, [this](int cur){
        mNoteWidget->setText(tr("Pointsize changed: %1 (Default: %2)").arg(QString::number(cur), "10"));
        mNoteWidget->setColor(qRgb(0, 0, 255));
    });
    connect(mOutputWidget->errListWidget(), SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onListWidgetDoubleClicked(QListWidgetItem*)));
    connect(mOutputWidget->outputListWidget(), SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onListWidgetDoubleClicked(QListWidgetItem*)));
    connect(mBtnParse, SIGNAL(clicked()), this, SLOT(onParse()));


    //创建布局
    QHBoxLayout *layBottom = new QHBoxLayout;
    layBottom->addWidget(mNoteWidget);
    layBottom->addWidget(mBtnParse, 0, Qt::AlignRight);
    bottomWidget->setLayout(layBottom);

    QVBoxLayout *layCentral = new QVBoxLayout;
    layCentral->setMargin(0);
    layCentral->setSpacing(0);
    layCentral->addSpacing(4);
    layCentral->addWidget(labPath);
    layCentral->addSpacing(4);
    layCentral->addWidget(mEdit);
    layCentral->addWidget(bottomWidget);
    mainWidget->setLayout(layCentral);

    mSplitter->addWidget(mainWidget);
    mSplitter->addWidget(mOutputWidget);
    mSplitter->setStretchFactor(0, 40);
    mSplitter->setStretchFactor(1, 1);
    mSplitter->setChildrenCollapsible(false);

    QHBoxLayout *layMain = new QHBoxLayout;
    layMain->setMargin(0);
    layMain->addWidget(mSplitter);
    setLayout(layMain);

    updateTr();
}

bool ProjWidget::load() {
    QFile file(mProjPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    mEdit->blockSignals(true);
    mEdit->setPlainText(in.readAll());
    mEdit->blockSignals(false);
    file.close();

    return true;
}

bool ProjWidget::save() {
    QFile file(mProjPath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << mEdit->toPlainText();
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

void ProjWidget::updateTr() {
    mBtnParse->setText(tr("Parse"));
}

void ProjWidget::onListWidgetDoubleClicked(QListWidgetItem *item) {
    ProjListWidgetItem *uItem = (ProjListWidgetItem*)item;
    emit processItemDbClick(uItem);
//     switch(item->data(Qt::UserRole).toInt()) {
//     case (int)UserRole::ShowPlainText: {
//         QPlainTextEdit *textWidget = new QPlainTextEdit;
//         textWidget->setWindowTitle(item->data(Qt::UserRole + 1).toString());
//         textWidget->setPlainText(item->data(Qt::UserRole + 2).toString());
//         textWidget->setReadOnly(true);
//         textWidget->setLineWrapMode(QPlainTextEdit::NoWrap);
//         textWidget->setAttribute(Qt::WA_DeleteOnClose);
//         textWidget->setMinimumSize(300, 300);
//         j::SetPointSize(textWidget, 11);
//         j::SetFamily(textWidget, fontSourceCodePro.mFamily);
//         textWidget->show();
//         break;
//     }
//     case (int)UserRole::ShowHtmlText: {
//         QTextEdit *textWidget = new QTextEdit;
//         textWidget->setWindowTitle(item->data(Qt::UserRole + 1).toString());
//         textWidget->setHtml(item->data(Qt::UserRole + 2).toString());
//         textWidget->setReadOnly(true);
//         textWidget->setLineWrapMode(QTextEdit::NoWrap);
//         textWidget->setAttribute(Qt::WA_DeleteOnClose);
//         textWidget->setMinimumSize(300, 300);
//         j::SetPointSize(textWidget, 11);
//         j::SetFamily(textWidget, fontSourceCodePro.mFamily);
//         textWidget->show();
//         break;
//     }
//     case (int)UserRole::MoveDocumentCursor: {
//         QPoint pos = item->data(Qt::UserRole + 1).toPoint();
//         if(pos.y() >= 0 && pos.y() < mEdit->document()->lineCount()) {
//             QTextBlock block = mEdit->document()->findBlockByLineNumber(pos.y());
//             QTextCursor tc = mEdit->textCursor();
//             tc.setPosition(block.position());
//             mEdit->setTextCursor(tc);
//             mEdit->setFocus();
//         }
//         break;
//     }
//     case (int)UserRole::OpenFolder: {
// #ifdef Q_OS_WIN
//         QString path = item->data(Qt::UserRole + 1).toString();
//         if(QDir().exists(path))
//             QProcess::startDetached("cmd.exe", QStringList() << "/c" << "start" << "" << path);
// #else
//         QMessageBox::information(this, "", tr("This function is not supported in this operating system"));
// #endif
//     }
//     }
}

void ProjWidget::onParse() {
    mBtnParse->setEnabled(false);

    mOutputWidget->clear();     //清除原有信息
    QListWidget *errListWidget = mOutputWidget->errListWidget();
    QListWidget *outputListWidget = mOutputWidget->outputListWidget();

    QTime t;    //用于计时（包括分析时间和输出文件时间）
    t.start();

    Parser parser(mProjPath, this);
    Parser::Result &result = parser.result();
    if(!result.issues().hasError()) {   //如果没有错误，则显示输出文件列表项
        QStringList paths = result.output();
        if(!paths.isEmpty()) {
            auto item = NewPlwiFn([paths] {
                QListWidget *widget = new QListWidget;
                
                //遍历所有输出文件路径，向widget添加item
                for(const QString &filePath : qAsConst(paths)) {
                    QListWidgetItem *item = new QListWidgetItem(filePath);
                    item->setIcon(QFileIconProvider().icon(filePath));
                    widget->addItem(item);
                }
                //绑定itemDoubleClicked，以打开相应文件夹
                connect(widget, &QListWidget::itemDoubleClicked, [](QListWidgetItem *item) {
                    QString path = QFileInfo(item->text()).path();
                    if(QDir().exists(path))
                        QDesktopServices::openUrl(QUrl("file:///" + path));
                });

                //设置窗口属性
                widget->setAttribute(Qt::WA_DeleteOnClose);
                widget->setWindowTitle(tr("Files"));
                widget->resize(600, 400);
                widget->setMinimumSize(300, 200);
                widget->show();
            });
            item->setText(tr("Files have been outputed"));
            outputListWidget->addItem(item);
        }
    }

    int ms = t.elapsed();
    ProjListWidgetItem *item = new ProjListWidgetItem(tr("Elapsed time: %1ms").arg(ms));
    item->setForeground(Qt::blue);
    outputListWidget->insertItem(0, item);

    mOutputWidget->setCurrentWidget(result.issues().hasError() ? errListWidget : outputListWidget);

    QString strRow = tr("Row:%1");
    QString strCol = tr("Col:%1");
    for(const Parser::Issue &issue : result.issues().list()) {  //遍历错误并显示到错误列表中
        //得到文本
        QString text;
        if(!issue.filePath.isEmpty())
            text += QFileInfo(issue.filePath).fileName() + ' ';
        if(issue.row != -1) {
            text += strRow.arg(issue.row + 1) + ' ';
            if(issue.col != -1)
                text += strCol.arg(issue.col + 1) + ' ';
        }

        //添加
        QListWidgetItem *item = issue.newItem();
        if(!item)
            item = new QListWidgetItem;
        item->setIcon(issue.icon());
        item->setText(text.isEmpty() ? issue.what : "[ " + text + "] " + issue.what);
        errListWidget->addItem(item);
    }

    QList<Parser::JSDebugMessage> jsDebugMsg = result.jsDebugMessage();
    if(!jsDebugMsg.isEmpty()) {     //如果有调试信息，则显示调试信息项
        Parser::ValueMap<Parser::CanonicalFilePath> files = result.files();
        auto item = NewPlwiFn([files, jsDebugMsg]{
            //lambda 用于显示调试信息
            auto fnShowMsg = [](const QString &fileName, const QString &text) {
                QPlainTextEdit *edit = new QPlainTextEdit;
                edit->setAttribute(Qt::WA_DeleteOnClose);
                edit->setWindowTitle(tr("Debug Message of \"%1\"").arg(fileName));
                edit->setPlainText(text);
                edit->resize(400, 340);
                j::SetFamily(edit, fontSourceCodePro.mFamily);
                j::SetPointSize(edit, 10);
                edit->show();
            };

            //如果只有一个文件有调试信息，则直接显示，否则弹出列表进行选择
            if(jsDebugMsg.size() == 1) {
                const Parser::JSDebugMessage &msg = jsDebugMsg[0];
                fnShowMsg(QFileInfo(files.indexKey(msg.fileId)).fileName(), msg.text);
            } else {
                QListWidget *widget = new QListWidget;

                //遍历所有调试信息，向widget添加item
                for(const Parser::JSDebugMessage &msg : qAsConst(jsDebugMsg)) {
                    QListWidgetItem *item = new QListWidgetItem(files.indexKey(msg.fileId));
                    item->setData(Qt::UserRole, msg.text);
                    widget->addItem(item);
                }
                //绑定itemDoubleClicked，以显示对应的调试信息
                connect(widget, &QListWidget::itemDoubleClicked, [fnShowMsg](QListWidgetItem *item) {
                    fnShowMsg(QFileInfo(item->text()).fileName(), item->data(Qt::UserRole).toString());
                });

                //设置窗口属性
                widget->setAttribute(Qt::WA_DeleteOnClose);
                widget->setWindowTitle(tr("Debug Message"));
                widget->resize(600, 400);
                widget->setMinimumSize(300, 200);
                widget->show();
            }
        });
        item->setText(tr("JS debug message"));
        outputListWidget->addItem(item);
    }

    /******.................

    if(!Parser::hasError()) {
        QString strDbClick = tr("(Double click to show detail)");

        if(Parser::hasOutputFile()) {
            QString strDir = Parser::outputDir(mProjPath);
            QListWidgetItem *item = new QListWidgetItem(tr("Files has been outputted to the directory \"%1\"").arg(strDir));
            item->setData(Qt::UserRole, (int)UserRole::OpenFolder);
            item->setData(Qt::UserRole + 1, strDir);
            outputListWidget->addItem(item);
        }

        // QString strProds = tr("Productions");
        QString strNil = tr("Empty string condition");
        QString strFirstSet = tr("FIRST set");
        QString strFollowSet = tr("FOLLOW set");
        QString strSelectSet = tr("SELECT set");

        // QListWidgetItem *itemProds = new QListWidgetItem(strProds + strDbClick);
        // itemProds->setData(Qt::UserRole, (int)UserRole::ShowPlainText);
        // itemProds->setData(Qt::UserRole + 1, strProds);
        // itemProds->setData(Qt::UserRole + 2, Parser::formatProdsMap());
        // outputListWidget->addItem(itemProds);

        QListWidgetItem *itemNils = new QListWidgetItem(strNil + strDbClick);
        itemNils->setData(Qt::UserRole, (int)UserRole::ShowPlainText);
        itemNils->setData(Qt::UserRole + 1, strNil);
        itemNils->setData(Qt::UserRole + 2, Parser::formatNilVec());
        outputListWidget->addItem(itemNils);

        QListWidgetItem *itemFirstSet = new QListWidgetItem(strFirstSet + strDbClick);
        itemFirstSet->setData(Qt::UserRole, (int)UserRole::ShowHtmlText);
        itemFirstSet->setData(Qt::UserRole + 1, strFirstSet);
        itemFirstSet->setData(Qt::UserRole + 2, Parser::formatFirstSet(true));
        outputListWidget->addItem(itemFirstSet);

        QListWidgetItem *itemFollowSet = new QListWidgetItem(strFollowSet + strDbClick);
        itemFollowSet->setData(Qt::UserRole, (int)UserRole::ShowHtmlText);
        itemFollowSet->setData(Qt::UserRole + 1, strFollowSet);
        itemFollowSet->setData(Qt::UserRole + 2, Parser::formatFollowSet(true));
        outputListWidget->addItem(itemFollowSet);

        QListWidgetItem *itemSelectSet = new QListWidgetItem(strSelectSet + strDbClick);
        itemSelectSet->setData(Qt::UserRole, (int)UserRole::ShowHtmlText);
        itemSelectSet->setData(Qt::UserRole + 1, strSelectSet);
        itemSelectSet->setData(Qt::UserRole + 2, Parser::formatSelectSet(true));
        outputListWidget->addItem(itemSelectSet);
    }*/

    mNoteWidget->setText(tr("Analysis completed"));
    mNoteWidget->setColor(qRgb(0, 128, 0));
    mBtnParse->setEnabled(true);
}

void ProjWidget::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}
