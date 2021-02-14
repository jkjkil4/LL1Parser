#pragma once

#include <QMenu>

class RFLMenu : public QMenu
{
    Q_OBJECT
public:
    explicit RFLMenu(QWidget *parent = nullptr);
    explicit RFLMenu(const QString &title, QWidget *parent = nullptr);
    ~RFLMenu() override;

    QAction *mActMoveToFirst = new QAction(tr("Move to first"));
    QAction *mActRemove = new QAction(tr("Remove"));
    QAction *mActShowInExplorer = new QAction(tr("Show in Explorer"));

private:
    void init();
};

