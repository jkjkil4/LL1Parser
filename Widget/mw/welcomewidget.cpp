#include "welcomeview.h"

WelcomeView::WelcomeView(QWidget *parent)
    : MainWindowView(parent), recentFiles(APP_DIR + "/config/RecentFiles.txt")
{

}

void WelcomeView::updateRecentFileList() {
    QStringList lRecentFiles = recentFiles.loadAll();

}

void WelcomeView::appendRecentFile(const QString &filePath) {

}
