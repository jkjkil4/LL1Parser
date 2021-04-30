#pragma once

#include <QJSEngine>
#include <QThread>
#include <QEventLoop>

class ThreadEvaluate : public QThread
{
    Q_OBJECT
    void run() override;
public:
    explicit ThreadEvaluate(QJSEngine *jsEngine, QObject *parent = nullptr);

    void setEvaluateArgs(const QString &program, const QString &fileName = QString(), int lineNumber = 1);

signals:
    void evaluateResultReady(const QJSValue &result);

private:
    QJSEngine *mJsEngine;

    QString mEvaluateProgram;
    QString mEvaluateFileName;
    int mEvaluateLineNumber;
};

class ThreadCall : public QThread
{
    Q_OBJECT
    void run() override;
public:
    explicit ThreadCall(QJSEngine *jsEngine, QObject *parent = nullptr);

    void setCallVal(const QJSValue &callVal);

signals:
    void callResultReady(const QJSValue &result);

private:
    QJSEngine *mJsEngine;

    QJSValue mCallVal;
};

class JSEngine : public QJSEngine
{
    Q_OBJECT
public:
    JSEngine(QObject *parent = nullptr);

    QJSValue terminableEvaluate(const QString &program, const QString &fileName = QString(), int lineNumber = 1);
    QJSValue terminableCall(const QJSValue &jsCallVal);

public slots:
    void onTerminateEvaluate();
    void onTerminateCall();

private slots:
    void onResultReady(const QJSValue &result);

private:
    ThreadEvaluate threadEvaluate;
    ThreadCall threadCall;
    QEventLoop eventLoop;

    QJSValue jsResult;
};
