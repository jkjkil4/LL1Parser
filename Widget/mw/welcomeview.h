#ifndef WELCOMEWIDGET_H
#define WELCOMEWIDGET_H

#include "mainwindowview.h"
#include <QLabel>
#include <QListWidget>

#include <RecentFileManager/recentfilemanager.h>

class WelcomeView : public MainWindowView
{
    Q_OBJECT
public:
    explicit WelcomeView(QWidget *parent = nullptr);
    ~WelcomeView() override = default;

    void updateRecentFileList();
    void appendRecentFile(const QString &filePath);

private:
    RecentFileManager recentFiles;  //"最近项目"相关

    QListWidget *listWidget = new QListWidget;  //"最近项目"列表
};

#endif // WELCOMEWIDGET_H
