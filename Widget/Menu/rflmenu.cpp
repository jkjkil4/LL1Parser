#include "rflmenu.h"

RFLMenu::RFLMenu(QWidget *parent) : QMenu(parent) { init(); }

RFLMenu::RFLMenu(const QString &title, QWidget *parent) : QMenu(title, parent) { init(); }

RFLMenu::~RFLMenu() {
    delete actMoveToFirst;
    delete actRemove;
    delete actShowInExplorer;
}

void RFLMenu::init() {
    addAction(actMoveToFirst);
    addAction(actRemove);
    addSeparator();
    addAction(actShowInExplorer);
}
