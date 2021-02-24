#include "parser.h"

QList<Parser::Issue> Parser::issues;
QMap<QString, Parser::Divided> Parser::mapDivided;
QMap<Parser::Symbol, int> Parser::mapSymbols;
int Parser::symbolsMaxIndex = 0;
int Parser::terminalMaxIndex = 0;

void Parser::divide(QTextDocument *doc) {
    QRegularExpression regExp("%\\[(.*?)\\]%");     //正则表达式
    int count = doc->lineCount();

    Divided *pDivided = nullptr;     //默认Divided
    repeat(int, i, count) {     //遍历文档的所有行
        QString line = doc->findBlockByLineNumber(i).text();    //当前行
        int start = 0;
        QRegularExpressionMatch match = regExp.match(line);     //使用正则表达式获取内容
        while(match.hasMatch()) {   //重复直到正则表达式获取不到内容
            QString mid = line.mid(start, match.capturedStart() - start);
            if(pDivided) pDivided->parts.append(Divided::Part{ i, mid });

            start = match.capturedEnd();
            pDivided = &mapDivided[match.captured(1)];
            match = regExp.match(line, match.capturedEnd());
        }
        QString right = line.right(line.length() - start);
        if(pDivided) pDivided->parts.append(Divided::Part{ i, right });
    }
}


#define TRY_PARSE(key, fn) {            \
    auto iter = mapDivided.find(key);   \
    if(iter != mapDivided.end()) {      \
        fn(*iter);                      \
        iter->parsed = true;            \
    }                                   \
}
void Parser::parse(QTextDocument *doc) {
    clear();
    divide(doc);

    TRY_PARSE("Terminal", parseTerminal);
    terminalMaxIndex = symbolsMaxIndex;
    TRY_PARSE("Nonterminal", parseNonterminal);
    TRY_PARSE("Production", parseProduction);

    for(auto iter = mapDivided.begin(); iter != mapDivided.end(); ++iter) {
        if(!iter->parsed) {
            issues << Issue(Issue::Error, tr("Unknown tag \"%1\"").arg(iter.key()));
        }
    }

//    int count = doc->lineCount();
//    repeat(int, i, count) {
//        QString line = doc->findBlockByLineNumber(i).text().simplified().trimmed();
//        QStringList list = line.split(' ', QString::SkipEmptyParts);
//        int size = list.size();
//        if(size < 2) {
//            if(size == 1)
//                issues.append(Issue(Issue::Error, tr("Lack of \"->\" to represent production"), i));
//            continue;
//        }
//        if(list[1] != "->") {
//            issues.append(Issue(Issue::Error, tr("Should be \"->\" instead of \"%1\"").arg(list[1]), i, 1));
//            continue;
//        }
//    }
}
#undef TRY_PARSE

void Parser::parseTerminal(const Divided &divided) {
    for(const Divided::Part &part : divided.parts) {    //遍历所有行
        QStringList list = part.text.split(' ', QString::SkipEmptyParts);   //分割字符串
        int phrase = -1;
        for(QString &str : list) {  //遍历分割的结果
            phrase++;
            auto iter = mapSymbols.find(str);
            if(iter != mapSymbols.end()) {  //如果该符号已经存在
                if(iter.key().type == Symbol::Nonterminal) {
                    issues << Issue(Issue::Error, tr("Symbol \"%1\" is already an nonterminal").arg(str), part.row, phrase);
                } else {
                    issues << Issue(Issue::Warning, tr("Redefinition of \"%1\"").arg(str), part.row, phrase);
                }
                continue;
            }
            mapSymbols.insert(Symbol(Symbol::Terminal, str), symbolsMaxIndex);
            symbolsMaxIndex++;
        }
    }
}

void Parser::parseNonterminal(const Divided &divided) {
    for(const Divided::Part &part : divided.parts) {    //遍历所有行
        QStringList list = part.text.split(' ', QString::SkipEmptyParts);   //分割字符串
        int phrase = -1;
        for(QString &str : list) {  //遍历分割的结果
            phrase++;
            auto iter = mapSymbols.find(str);
            if(iter != mapSymbols.end()) {  //如果该符号已经存在
                if(iter.key().type == Symbol::Terminal) {
                    issues << Issue(Issue::Error, tr("Symbol \"%1\" is already a terminal").arg(str), part.row, phrase);
                } else {
                    issues << Issue(Issue::Warning, tr("Redefinition of \"%1\"").arg(str), part.row, phrase);
                }
                continue;
            }
            mapSymbols.insert(Symbol(Symbol::Nonterminal, str), symbolsMaxIndex);
            symbolsMaxIndex++;
        }
    }
}

void Parser::parseProduction(const Divided &divided) {
    Q_UNUSED(divided)
}

void Parser::clear() {
    issues.clear();
    mapDivided.clear();
    mapSymbols.clear();
    symbolsMaxIndex = 0;
}

bool Parser::hasError() {
    for(Issue &issue : issues)
        if(issue.type == Issue::Error)
            return true;
    return false;
}


