#pragma once

#include <QObject>

class JSObject : public QObject
{
    Q_OBJECT
public:
    void clear();

    bool hasDebugMessage() { return !mDebugMessage.isEmpty(); }
    QString debugMessage() { return mDebugMessage; }

public slots:
    void debug(const QString &text);

private:
    QString mDebugMessage;
};
