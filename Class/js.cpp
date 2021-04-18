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


JSEngine::JSEngine(QObject *parent) : QJSEngine(parent), threadEvaluate(this) {
    connect(&threadEvaluate, &ThreadEvaluate::evaluateResultReady, this, &JSEngine::onEvaluateResultReady);
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

void JSEngine::onTerminateProcess() {
    threadEvaluate.terminate();
    threadEvaluate.wait();
    eventLoop.quit();
}
void JSEngine::onEvaluateResultReady(const QJSValue &result) {
    jsResult = result;
    eventLoop.quit();
}
