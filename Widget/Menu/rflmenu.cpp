#include "rflmenu.h"

RFLMenu::RFLMenu(QWidget *parent) : QMenu(parent) { init(); }

RFLMenu::RFLMenu(const QString &title, QWidget *parent) : QMenu(title, parent) { init(); }

RFLMenu::~RFLMenu() {
    delete mActMoveToFirst;
    delete mActRemove;
    delete mActShowInExplorer;
}

void RFLMenu::init() {
    addAction(mActMoveToFirst);
    addAction(mActRemove);
    addSeparator();
    addAction(mActShowInExplorer);
}
