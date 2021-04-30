#pragma once

#include <QPlainTextEdit>
#include <QTextDocument>
#include <QTextBlock>

class ItemData
{
public:
    virtual ~ItemData() = default;

    virtual void onDoubleClicked() {}
};

class ItemData_MoveDocCursor : public ItemData
{
public:
    ItemData_MoveDocCursor(QPlainTextEdit *edit, int row, int col);

    void onDoubleClicked() override;

private:
    QPlainTextEdit *mEdit;
    int mRow, mCol;
};
