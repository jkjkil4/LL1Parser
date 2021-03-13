#pragma once

#include <QObject>

#include <Lib/header.h>

class JSObject : public QObject
{
    Q_OBJECT
public:
    void clear();

    bool hasDebugMessage() { return !mDebugMessage.isEmpty(); }
    QString debugMessage() { return mDebugMessage; }

    VAR_FUNC(nonterminalMaxIndex, setNonterminalMaxIndex, mNonterminalMaxIndex, int, , )
    VAR_FUNC(terminalMaxIndex, setTerminalMaxIndex, mTerminalMaxIndex, int, , )

public slots:
    bool isNonterminal(int symbol);
    bool isTerminal(int symbol);

    void debug(const QString &text);

private:
    int mNonterminalMaxIndex = 0;
    int mTerminalMaxIndex = 0;

    QString mDebugMessage;
};
