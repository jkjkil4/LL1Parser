#include "welcomewidget.h"
#include <QCoreApplication>
#include "RecentFileListWidget/rfldelegate.h"

WelcomeWidget::WelcomeWidget(QWidget *parent)
    : MainWindowView(parent)/*, recentFiles(APP_DIR + "/config/RecentFiles.txt")*/
{

    rflWidget->resize(400, 300);
    rflWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    rflWidget->setVerticalScrollMode(QListView::ScrollPerPixel);
}

void WelcomeWidget::updateRecentFileList() {
//    QStringList lRecentFiles = recentFiles.loadAll();

}

void WelcomeWidget::appendRecentFile(const QString &filePath) {

}
