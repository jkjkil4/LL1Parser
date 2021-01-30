#ifndef RFLMENU_H
#define RFLMENU_H

#include <QMenu>

class RFLMenu : public QMenu
{
    Q_OBJECT
public:
    explicit RFLMenu(QWidget *parent = nullptr);
    explicit RFLMenu(const QString &title, QWidget *parent = nullptr);
    ~RFLMenu() override;

    QAction *actMoveToFirst = new QAction(tr("Move to first"));
    QAction *actRemove = new QAction(tr("Remove"));
    QAction *actShowInExplorer = new QAction(tr("Show in Explorer"));

private:
    void init();
};

#endif // RFLMENU_H
