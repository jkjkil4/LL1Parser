#include "projwidget.h"

ProjWidget::ProjWidget(const QString &projPath, QWidget *parent)
    : QWidget(parent), projName(QFileInfo(projPath).completeBaseName()), projPath(projPath)
{
    j::SetFamily(edit, fontSourceCodePro.family);
    j::SetPointSize(edit, 10);

    QLabel *labPath = new QLabel(projPath);
    labPath->setAlignment(Qt::AlignLeft);
    j::SetPointSize(labPath, 8);

    ColorWidget *bottomWidget = new ColorWidget;

    connect(edit, &QPlainTextEdit::textChanged, [this]{ setSaved(false); });


    //创建布局
    QHBoxLayout *layBtn = new QHBoxLayout;
    layBtn->addWidget(btnParse, 0, Qt::AlignRight);
    bottomWidget->setLayout(layBtn);

    QVBoxLayout *layMain = new QVBoxLayout;
    layMain->setMargin(0);
    layMain->setSpacing(0);
    layMain->addSpacing(4);
    layMain->addWidget(labPath);
    layMain->addSpacing(4);
    layMain->addWidget(edit);
    layMain->addWidget(bottomWidget);
    setLayout(layMain);


    updateTr();
}

bool ProjWidget::load() {
    QFile file(projPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    edit->blockSignals(true);
    edit->setPlainText(in.readAll());
    edit->blockSignals(false);
    file.close();

    return true;
}

bool ProjWidget::save() {
    QFile file(projPath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << edit->toPlainText();
    file.close();

    setSaved(true);

    return true;
}

void ProjWidget::setSaved(bool _isSaved) {
    if(_isSaved != isSaved) {
        isSaved = _isSaved;
        emit stateChanged(_isSaved);
    }
}

void ProjWidget::updateTr() {
    btnParse->setText(tr("Parse"));
}

void ProjWidget::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}
