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
    });
//    connect(mOutputWidget->errListWidget(), SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onErrListWidgetDoubleClicked(QListWidgetItem*)));
//    connect(mOutputWidget->outputListWidget(), SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onOutputListWidgetDoubleClicked(QListWidgetItem*)));
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

//void ProjWidget::onErrListWidgetDoubleClicked(QListWidgetItem *item) {
//    //得到相关数据
//    QPoint pos = item->data(Qt::UserRole).toPoint();
//    if(pos.y() != -1 && pos.y() <= mEdit->document()->lineCount()) {
//        QTextCursor tc = mEdit->textCursor();
//        tc.setPosition(mEdit->document()->findBlockByLineNumber(pos.y()).position());
//        mEdit->setTextCursor(tc);
//        mEdit->setFocus();
//    }
//}

//void ProjWidget::onOutputListWidgetDoubleClicked(QListWidgetItem *item) {
//    if(item->data(Qt::UserRole).toInt() == (int)UserRole::ShowPlainText) {
//        QPlainTextEdit *textWidget = new QPlainTextEdit;
//        textWidget->setWindowTitle(item->data(Qt::UserRole + 1).toString());
//        textWidget->setPlainText(item->data(Qt::UserRole + 2).toString());
//        textWidget->setReadOnly(true);
//        textWidget->setLineWrapMode(QPlainTextEdit::NoWrap);
//        textWidget->setAttribute(Qt::WA_DeleteOnClose);
//        textWidget->setMinimumSize(300, 300);
//        j::SetPointSize(textWidget, 11);
//        j::SetFamily(textWidget, fontSourceCodePro.mFamily);
//        textWidget->show();
//    }
//}

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
        QString strProds = tr("Productions");
        QString strNil = tr("Empty string state");
        QString strDbClick = tr("(Double click to show detail)");

        QListWidgetItem *itemProds = new QListWidgetItem(strProds + strDbClick);
        itemProds->setData(Qt::UserRole, (int)UserRole::ShowPlainText);
        itemProds->setData(Qt::UserRole + 1, strProds);
        itemProds->setData(Qt::UserRole + 2, Parser::formatProdsMap());
        outputListWidget->addItem(itemProds);

        QListWidgetItem *itemNils = new QListWidgetItem(strNil + strDbClick);
        itemNils->setData(Qt::UserRole, (int)UserRole::ShowPlainText);
        itemNils->setData(Qt::UserRole + 1, strNil);
        itemNils->setData(Qt::UserRole + 2, Parser::formatNilVec());
        outputListWidget->addItem(itemNils);
    }
}

void ProjWidget::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}
