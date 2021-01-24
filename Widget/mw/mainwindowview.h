#pragma once
#include <QWidget>

class MainWindowView : public QWidget
{
    Q_OBJECT
public:
    using QWidget::QWidget;

signals:
    void changeView(const QString &viewName);
};
