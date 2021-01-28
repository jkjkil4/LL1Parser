#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QWidget>

#include <Lib/header.h>
#include <Lib/paint.h>

class InfoWidget : public QWidget
{
    Q_OBJECT
protected:
    void paintEvent(QPaintEvent *) override;

public:
    explicit InfoWidget(QWidget *parent = nullptr);
    explicit InfoWidget(const QString &text, QColor col, QWidget *parent = nullptr);

    void setData(const QString &_text, QColor _col);
    void clear();
    void setPointSize(int pointSize);

private:
    void updateHeight();

    bool paint = false;
    QString text;
    QColor col;
};

#endif // INFOWIDGET_H
