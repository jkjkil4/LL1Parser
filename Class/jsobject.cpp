#include "jsobject.h"
#include <QDebug>

void JSObject::clear() {
    mDebugMessage.clear();
}

bool JSObject::isNonterminal(int symbol) {
    return symbol >= 0 && symbol <= mNonterminalMaxIndex;
}
bool JSObject::isTerminal(int symbol) {
    return symbol > mNonterminalMaxIndex && symbol <= mTerminalMaxIndex;
}

void JSObject::debug(const QString &text) {
    mDebugMessage += text;
}
