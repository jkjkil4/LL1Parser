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
    connect(mOutputWidget->errListWidget(), SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onErrListWidgetDoubleClicked(QListWidgetItem*)));
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

void ProjWidget::onErrListWidgetDoubleClicked(QListWidgetItem *item) {
    //得到相关数据
    QPoint pos = item->data(Qt::UserRole).toPoint();
    if(pos.y() != -1 && pos.y() <= mEdit->document()->lineCount()) {
        QTextCursor tc = mEdit->textCursor();
        tc.setPosition(mEdit->document()->findBlockByLineNumber(pos.y()).position());
        mEdit->setTextCursor(tc);
        mEdit->setFocus();
    }
}

void ProjWidget::onParse() {
    QListWidget *errListWidget = mOutputWidget->errListWidget();
    errListWidget->clear();

    Parser::parse(mEdit->document());
    if(!Parser::issues.isEmpty()) {   //如果有错误
        mOutputWidget->setCurrentWidget(errListWidget);     //设置mOutputWidget当前显示的控件为errListWidget

        QString strRow = tr("Row");
        QString strPhrase = tr("Phrase");

        for(Parser::Issue &issue : Parser::issues) {    //遍历所有错误
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
            item->setData(Qt::UserRole, QPoint(issue.phrase, issue.row));
            item->setIcon(issue.icon());
            errListWidget->addItem(item);
        }
    }
}

void ProjWidget::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}
