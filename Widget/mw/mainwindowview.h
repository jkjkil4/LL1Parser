#ifndef MAINWINDOWWIDGET_H
#define MAINWINDOWWIDGET_H

#include <QWidget>

class MainWindowView : public QWidget
{
    Q_OBJECT
public:
    using QWidget::QWidget;

signals:
    void changeView(const QString &viewName);
};

#endif // MAINWINDOWWIDGET_H
