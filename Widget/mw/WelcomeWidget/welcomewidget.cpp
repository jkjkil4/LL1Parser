#include "welcomewidget.h"
#include <QCoreApplication>

#include <QDebug>

WelcomeWidget::WelcomeWidget(QWidget *parent)
    : MainWindowView(parent)/*, recentFiles(APP_DIR + "/config/RecentFiles.txt")*/
{
    rflWidget->setPath(APP_DIR + "/Config/rfl.txt");
    rflWidget->updateList();
    rflWidget->resize(400, 300);
    rflWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    rflWidget->setVerticalScrollMode(QListView::ScrollPerPixel);
    //connect(rflWidget, &RFLWidget::itemClicked, [](const RFLWidget::Item &item){ qDebug() << item.row() << item.name() << item.path(); });
}

void WelcomeWidget::updateRecentFileList() {
//    QStringList lRecentFiles = recentFiles.loadAll();

}

void WelcomeWidget::appendRecentFile(const QString &filePath) {

}
