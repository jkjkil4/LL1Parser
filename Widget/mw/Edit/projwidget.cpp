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

    connect(mEdit, &PlainTextEdit::textChanged, [this]{ setSaved(false); });
    connect(mEdit, &PlainTextEdit::pointSizeChanged, [this](int cur){
        mNoteWidget->setText(tr("Pointsize changed: %1 (Default: %2)").arg(QString::number(cur), "10"));
    });
    connect(mBtnParse, SIGNAL(clicked()), this, SLOT(onParse()));


    //创建布局
    QHBoxLayout *layBottom = new QHBoxLayout;
    layBottom->addWidget(mNoteWidget);
    layBottom->addWidget(mBtnParse, 0, Qt::AlignRight);
    bottomWidget->setLayout(layBottom);

    QVBoxLayout *layMain = new QVBoxLayout;
    layMain->setMargin(0);
    layMain->setSpacing(0);
    layMain->addSpacing(4);
    layMain->addWidget(labPath);
    layMain->addSpacing(4);
    layMain->addWidget(mEdit);
    layMain->addWidget(bottomWidget);
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

void ProjWidget::onParse() {
    Parser::parse(mEdit->document());

}

void ProjWidget::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}
