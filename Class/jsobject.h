#pragma once

#include <QObject>

#include <Lib/header.h>

class JSObject : public QObject
{
    Q_OBJECT
public:
    void clear();   //清空内容

    bool hasDebugMessage() { return !mDebugMessage.isEmpty(); }     //返回是否有调试信息
    QString debugMessage() const { return mDebugMessage; }    //返回调试信息文本

    VAR_FUNC(nonterminalMaxIndex, setNonterminalMaxIndex, mNonterminalMaxIndex, int, , )
    VAR_FUNC(terminalMaxIndex, setTerminalMaxIndex, mTerminalMaxIndex, int, , )

public slots:
    bool isNonterminal(int symbol);     //判断是否是非终结符
    bool isTerminal(int symbol);        //判断是否是终结符

    void debug(const QString &text);    //输出调试信息

private:
    //用于非终结符和终结符的判断
    int mNonterminalMaxIndex = 0;
    int mTerminalMaxIndex = 0;

    QString mDebugMessage;  //调试信息
};
