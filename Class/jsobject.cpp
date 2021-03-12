#include "jsobject.h"
#include <QDebug>

void JSObject::clear() {
    mDebugMessage.clear();
}

void JSObject::debug(const QString &text) {
    mDebugMessage += text;
}
