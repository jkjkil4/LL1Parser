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
    switch(item->data(Qt::UserRole).toInt()) {
    case (int)UserRole::ShowPlainText: {
        QPlainTextEdit *textWidget = new QPlainTextEdit;
        textWidget->setWindowTitle(item->data(Qt::UserRole + 1).toString());
        textWidget->setPlainText(item->data(Qt::UserRole + 2).toString());
        textWidget->setReadOnly(true);
        textWidget->setLineWrapMode(QPlainTextEdit::NoWrap);
        textWidget->setAttribute(Qt::WA_DeleteOnClose);
        textWidget->setMinimumSize(300, 300);
        j::SetPointSize(textWidget, 11);
        j::SetFamily(textWidget, fontSourceCodePro.mFamily);
        textWidget->show();
        break;
    }
    case (int)UserRole::ShowHtmlText: {
        QTextEdit *textWidget = new QTextEdit;
        textWidget->setWindowTitle(item->data(Qt::UserRole + 1).toString());
        textWidget->setHtml(item->data(Qt::UserRole + 2).toString());
        textWidget->setReadOnly(true);
        textWidget->setLineWrapMode(QTextEdit::NoWrap);
        textWidget->setAttribute(Qt::WA_DeleteOnClose);
        textWidget->setMinimumSize(300, 300);
        j::SetPointSize(textWidget, 11);
        j::SetFamily(textWidget, fontSourceCodePro.mFamily);
        textWidget->show();
        break;
    }
    case (int)UserRole::MoveDocumentCursor: {
        QPoint pos = item->data(Qt::UserRole + 1).toPoint();
        if(pos.y() >= 0 && pos.y() < mEdit->document()->lineCount()) {
            QTextBlock block = mEdit->document()->findBlockByLineNumber(pos.y());
            QTextCursor tc = mEdit->textCursor();
            tc.setPosition(block.position());
            mEdit->setTextCursor(tc);
            mEdit->setFocus();
        }
        break;
    }
    case (int)UserRole::OpenFolder: {
#ifdef Q_OS_WIN
        QString path = item->data(Qt::UserRole + 1).toString();
        if(QDir().exists(path))
            QProcess::startDetached("cmd.exe", QStringList() << "/c" << "start" << "" << path);
#else
        QMessageBox::information(this, "", tr("This function is not supported in this operating system"));
#endif
    }
    }
}

void ProjWidget::onParse() {
    QListWidget *errListWidget = mOutputWidget->errListWidget();
    QListWidget *outputListWidget = mOutputWidget->outputListWidget();
    errListWidget->clear();
    outputListWidget->clear();

    QTime t;
    t.start();

    Parser::parse(mEdit->document());
    if(!Parser::hasError())
        Parser::outputFile(mProjPath);

    int ms = t.elapsed();
    QListWidgetItem *item = new QListWidgetItem(tr("Elapsed time: %1ms").arg(ms));
    item->setForeground(Qt::blue);
    item->setData(Qt::UserRole, (int)UserRole::NoRole);
    outputListWidget->addItem(item);

    mOutputWidget->setCurrentWidget(Parser::issues.isEmpty() ? outputListWidget : errListWidget);

    QString strRow = tr("Row");
    QString strPhrase = tr("Phrase");
    for(Parser::Issue &issue : Parser::issues) {    //遍历所有问题
        //得到文本
        QString text;
        if(issue.row != -1) {
            text += strRow + ":" + QString::number(issue.row + 1) + "    ";
            if(issue.phrase != -1)
                text += strPhrase + ":" + QString::number(issue.phrase + 1) + "    ";
        }
        text += issue.what;

        //添加
        QListWidgetItem *item = new QListWidgetItem(text);
        if(issue.userDataList.isEmpty()) {
            if(issue.row == -1) {
                item->setData(Qt::UserRole, (int)UserRole::NoRole);
            } else {
                item->setData(Qt::UserRole, (int)UserRole::MoveDocumentCursor);
                item->setData(Qt::UserRole + 1, QPoint(qMax(0, issue.phrase), issue.row));
            }
        } else {
            int offset = 0;
            for(QVariant &variant : issue.userDataList) {
                item->setData(Qt::UserRole + offset, variant);
                offset++;
            }
        }
        item->setIcon(issue.icon());
        errListWidget->addItem(item);
    }
    if(!Parser::hasError()) {
        QString strDbClick = tr("(Double click to show detail)");

        if(!Parser::jsDebugMessage.isEmpty()) {
            QString strJSDebugMsg = tr("JS debug message");
            QListWidgetItem *itemJSDebug = new QListWidgetItem(strJSDebugMsg + strDbClick);
            itemJSDebug->setData(Qt::UserRole, (int)UserRole::ShowPlainText);
            itemJSDebug->setData(Qt::UserRole + 1, strJSDebugMsg);
            itemJSDebug->setData(Qt::UserRole + 2, Parser::jsDebugMessage);
            outputListWidget->addItem(itemJSDebug);
        }

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
    }

    mNoteWidget->setText(tr("Analysis completed"));
    mNoteWidget->setColor(qRgb(0, 128, 0));
}

void ProjWidget::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}
