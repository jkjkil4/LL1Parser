#include "projlistwidgetitem.h"

#include "Widget/mw/Edit/editview.h"
#include "Widget/mw/Edit/projwidget.h"

//-------------ProjListWidgetItem---------------
void ProjListWidgetItem::onDoubleClicked(EditView *) {}
ProjListWidgetItem* ProjListWidgetItem::copy() const { return new ProjListWidgetItem(*this); }

//-------------PLWI_MoveDocCursor---------------
void PLWI_MoveDocCursor::onDoubleClicked(EditView *editView) {
    QPlainTextEdit *edit = editView->open(projPath)->editWidget();
    QTextDocument *doc = edit->document();
    if(mRow < 0 || mRow >= doc->lineCount())
        return;
    QTextBlock block = doc->findBlockByLineNumber(mRow);
    if(mCol < 0 || mCol > block.length())
        return;
    QTextCursor tc = edit->textCursor();
    tc.setPosition(block.position() + mCol);
    edit->setTextCursor(tc);
    edit->setFocus();
}

//---------------PLWI_ShowPlainText--------------
void PLWI_ShowPlainText::onDoubleClicked(EditView *) {
    QPlainTextEdit *edit = new QPlainTextEdit;
    edit->setAttribute(Qt::WA_DeleteOnClose);
    edit->setWindowTitle(title);
    edit->setReadOnly(true);
    edit->setPlainText(text);
    edit->resize(600, 400);
    edit->setMinimumSize(300, 200);
    j::SetPointSize(edit, 10);
    j::SetFamily(edit, fontSourceCodePro.mFamily);
    edit->show();
}

//----------------PLWI_ShowHtmlText--------------
void PLWI_ShowHtmlText::onDoubleClicked(EditView *) {
    QTextEdit *edit = new QTextEdit;
    edit->setAttribute(Qt::WA_DeleteOnClose);
    edit->setWindowTitle(title);
    edit->setReadOnly(true);
    edit->setHtml(html);
    edit->resize(600, 400);
    edit->setMinimumSize(300, 200);
    j::SetPointSize(edit, 10);
    j::SetFamily(edit, fontSourceCodePro.mFamily);
    edit->show();
}