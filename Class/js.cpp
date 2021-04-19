#include "js.h"

#include <QDebug>

ThreadEvaluate::ThreadEvaluate(QJSEngine *jsEngine, QObject *parent) 
    : QThread(parent), mJsEngine(jsEngine) {} 

void ThreadEvaluate::setEvaluateArgs(const QString &program, const QString &fileName, int lineNumber) {
    mEvaluateProgram = program;
    mEvaluateFileName = fileName;
    mEvaluateLineNumber = lineNumber;
}
void ThreadEvaluate::run() {
    QJSValue result = mJsEngine->evaluate(mEvaluateProgram, mEvaluateFileName, mEvaluateLineNumber);
    emit evaluateResultReady(result);
}


ThreadCall::ThreadCall(QJSEngine *jsEngine, QObject *parent)
    : QThread(parent), mJsEngine(jsEngine) {}

void ThreadCall::setCallVal(const QJSValue &callVal) {
    mCallVal = callVal;
}
void ThreadCall::run() {
    QJSValue result = mCallVal.call();
    emit callResultReady(result);
}


JSEngine::JSEngine(QObject *parent) : QJSEngine(parent), threadEvaluate(this), threadCall(this) {
    connect(&threadEvaluate, &ThreadEvaluate::evaluateResultReady, this, &JSEngine::onResultReady);
    connect(&threadCall, &ThreadCall::callResultReady, this, &JSEngine::onResultReady);
}

QJSValue JSEngine::terminableEvaluate(const QString &program, const QString &fileName, int lineNumber) {
    jsResult = QJSValue();
    threadEvaluate.setEvaluateArgs(program, fileName, lineNumber);
    threadEvaluate.start();
    eventLoop.exec();
    threadEvaluate.quit();
    threadEvaluate.wait();
    return jsResult;
}
QJSValue JSEngine::terminableCall(const QJSValue &jsCallVal) {
    jsResult = QJSValue();
    threadCall.setCallVal(jsCallVal);
    threadCall.start();
    eventLoop.exec();
    threadCall.quit();
    threadCall.wait();
    return jsResult;
}

void JSEngine::onTerminateEvaluate() {
    threadEvaluate.terminate();
    threadEvaluate.wait();
    eventLoop.quit();
}
void JSEngine::onTerminateCall() {
    threadCall.terminate();
    threadCall.wait();
    eventLoop.quit();
}
void JSEngine::onResultReady(const QJSValue &result) {
    jsResult = result;
    eventLoop.quit();
}
