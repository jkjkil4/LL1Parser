#pragma once

#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QTextBlock>

class EditView;
class ProjWidget;

class ProjListWidgetItem : public QListWidgetItem
{
public:
    using QListWidgetItem::QListWidgetItem;

    virtual ProjListWidgetItem* copy() const;

    virtual void onDoubleClicked(EditView *editView);
    virtual void onDoubleClicked(QPlainTextEdit *edit);
};

//--------------PLWI_MoveDocCursor--------------
class PLWI_MoveDocCursor : public ProjListWidgetItem
{
public:
    PLWI_MoveDocCursor(const QString &projPath, int row, int col);

    ProjListWidgetItem* copy() const override;

    void onDoubleClicked(EditView *editView) override;
    void onDoubleClicked(QPlainTextEdit *edit) override;

private:
    QString projPath;
    int mRow, mCol;
};

//-------------PLWI_Fn---------------
template<typename Fn>
class PLWI_Fn : public ProjListWidgetItem
{
public:
    PLWI_Fn(Fn fn) : fn(fn) {}

    ProjListWidgetItem* copy() const override { return new PLWI_Fn<Fn>(*this); }

    void onDoubleClicked(EditView *) override { fn(); }

private:
    Fn fn;
};

template<typename Fn>
inline PLWI_Fn<Fn>* NewPlwiFn(Fn fn) { return new PLWI_Fn<Fn>(fn); }
