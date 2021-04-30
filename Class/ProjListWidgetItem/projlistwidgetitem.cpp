#include "projlistwidgetitem.h"

#include "Widget/mw/Edit/editview.h"
#include "Widget/mw/Edit/projwidget.h"

//-------------copy---------------
ProjListWidgetItem* ProjListWidgetItem::copy() const { return new ProjListWidgetItem(*this); }
ProjListWidgetItem* PLWI_MoveDocCursor::copy() const { return new PLWI_MoveDocCursor(*this); }

//-------------ProjListWidgetItem---------------
void ProjListWidgetItem::onDoubleClicked(EditView *) {}
void ProjListWidgetItem::onDoubleClicked(QPlainTextEdit *) {}

//-------------PLWI_MoveDocCursor---------------
PLWI_MoveDocCursor::PLWI_MoveDocCursor(const QString &projPath, int row, int col)
    : projPath(projPath), mRow(row), mCol(col == -1 ? 0 : col) {}
void PLWI_MoveDocCursor::onDoubleClicked(EditView *editView) {
    onDoubleClicked(editView->open(projPath)->editWidget());
}
void PLWI_MoveDocCursor::onDoubleClicked(QPlainTextEdit *edit) {
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
