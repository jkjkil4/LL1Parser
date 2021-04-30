#include "itemdata.h"

//------------------ItemData-MoveDocCursor------------------
ItemData_MoveDocCursor::ItemData_MoveDocCursor(QPlainTextEdit *edit, int row, int col)
    : mEdit(edit), mRow(row), mCol(col == -1 ? 0 : col) {}

void ItemData_MoveDocCursor::onDoubleClicked() {
    QTextDocument *doc = mEdit->document();
    if(mRow < 0 || mRow >= doc->lineCount())
        return;
    QTextBlock block = doc->findBlockByLineNumber(mRow);
    if(mCol < 0 || mCol > block.length())
        return;
    QTextCursor tc = mEdit->textCursor();
    tc.setPosition(block.position() + mCol);
    mEdit->setTextCursor(tc);
    mEdit->setFocus();
}
