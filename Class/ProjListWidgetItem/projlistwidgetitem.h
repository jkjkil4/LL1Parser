#pragma once

#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTextBlock>

class EditView;
class ProjWidget;

class ProjListWidgetItem : public QListWidgetItem
{
public:
    using QListWidgetItem::QListWidgetItem;

    virtual ProjListWidgetItem* copy() const;
    virtual void onDoubleClicked(EditView *editView);
};

//--------------PLWI_MoveDocCursor--------------
class PLWI_MoveDocCursor : public ProjListWidgetItem
{
public:
    PLWI_MoveDocCursor(const QString &projPath, int row, int col)
        : projPath(projPath), mRow(row), mCol(col == -1 ? 0 : col) {}

    ProjListWidgetItem* copy() const override { return new PLWI_MoveDocCursor(*this); }
    void onDoubleClicked(EditView *editView) override;

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

//--------------PLWI_ShowPlainText-------------
class PLWI_ShowPlainText : public ProjListWidgetItem
{
public:
    PLWI_ShowPlainText(const QString &title, const QString &text) : title(title), text(text) {}

    ProjListWidgetItem* copy() const override { return new PLWI_ShowPlainText(*this); }
    void onDoubleClicked(EditView *) override;

private:
    QString title, text;
};

//--------------PLWI_ShowHtmlText---------------
class PLWI_ShowHtmlText : public ProjListWidgetItem
{
public:
    PLWI_ShowHtmlText(const QString &title, const QString &html) : title(title), html(html) {}

    ProjListWidgetItem* copy() const override { return new PLWI_ShowHtmlText(*this); }
    void onDoubleClicked(EditView *) override;

private:
    QString title, html;
};
