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

class JSEngine : public QJSEngine
{
    Q_OBJECT
public:
    JSEngine(QObject *parent = nullptr);

    QJSValue terminableEvaluate(const QString &program, const QString &fileName = QString(), int lineNumber = 1);

public slots:
    void onTerminateProcess();

private slots:
    void onEvaluateResultReady(const QJSValue &result);

private:
    ThreadEvaluate threadEvaluate;
    QEventLoop eventLoop;

    QJSValue jsResult;
};
