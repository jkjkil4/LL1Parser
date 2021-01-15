#ifndef WELCOMEWIDGET_H
#define WELCOMEWIDGET_H

#include "mainwindowview.h"
#include <QLabel>
#include <QListWidget>

#include "RecentFileListWidget/recentfilelistwidget.h"

class WelcomeWidget : public MainWindowView
{
    Q_OBJECT
public:
    explicit WelcomeWidget(QWidget *parent = nullptr);
    ~WelcomeWidget() override = default;

    void updateRecentFileList();
    void appendRecentFile(const QString &filePath);

private:
    RFLWidget *rflWidget = new RFLWidget(this);
};

#endif // WELCOMEWIDGET_H
