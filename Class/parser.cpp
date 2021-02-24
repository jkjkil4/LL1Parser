#include "parser.h"

QList<Parser::Issue> Parser::issues;
QMap<QString, Parser::Divided> Parser::mapDivided;
QMap<Parser::Symbol, int> Parser::mapSymbols;
int Parser::symbolsMaxIndex = 0;
int Parser::nonterminalMaxIndex = 0;
Parser::ProdsMap Parser::mapProds;

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
            if(pDivided) pDivided->parts << Divided::Part{ i, mid };

            start = match.capturedEnd();
            pDivided = &mapDivided[match.captured(1)];
            pDivided->rows << i;
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

    appendSymbol(Symbol::Nonterminal, "S");
    TRY_PARSE("Nonterminal", parseNonterminal);
    nonterminalMaxIndex = symbolsMaxIndex;
    TRY_PARSE("Terminal", parseTerminal);

    TRY_PARSE("Production", parseProduction);

    QString trStr = tr("Unknown tag \"%1\"");
    for(auto iter = mapDivided.begin(); iter != mapDivided.end(); ++iter) {
        if(!iter->parsed) {
            for(int row : iter->rows) {
                issues << Issue(Issue::Error, trStr.arg(iter.key()), row);
            }
        }
    }
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
            appendSymbol(Symbol::Terminal, str);
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
            appendSymbol(Symbol::Nonterminal, str);
        }
    }
}

void Parser::parseProduction(const Divided &divided) {
    for(const Divided::Part &part : divided.parts) {    //遍历所有行
        QStringList list = part.text.split(' ', QString::SkipEmptyParts);   //分割字符串
        int size = list.size();
        if(size < 2) {
            if(size == 1)
                issues << Issue(Issue::Error, tr("Lack of \"->\" to represent production"), part.row);
            continue;
        }
        if(list[1] != "->") {
            issues << Issue(Issue::Error, tr("Should be \"->\" instead of \"%1\"").arg(list[1]), part.row, 1);
            continue;
        }

        bool hasErr = false;
        Prod prod;
        //得到产生式的左部
        int leftDigit = mapSymbols.value(list[0], -1);
        if(leftDigit == -1) {
            issues << Issue(Issue::Error, tr("Unknown symbol \"%1\"").arg(list[0]), part.row, 0);
            hasErr = true;
        } else if(isTerminal(leftDigit)) {
            issues << Issue(Issue::Error, tr("The left of the production cannot be a terminal"), part.row, 0);
        }
        //得到产生式的右部
        int phrase = 1;
        for(auto iter = list.begin() + 2; iter != list.end(); ++iter) {     //从第三个开始遍历
            phrase++;
            int digit = mapSymbols.value(*iter, -1);
            if(digit == -1) {
                issues << Issue(Issue::Error, tr("Unknown symbol \"%1\"").arg(*iter), part.row, phrase);
                hasErr = true;
                continue;
            }
            if(!hasErr) prod << digit;
        }
        //如果没有错误，则将该产生式的右部附加到产生式左部符号的Prods中
        if(!hasErr) mapProds[leftDigit] << prod;
    }
}

void Parser::clear() {
    issues.clear();
    mapDivided.clear();
    mapSymbols.clear();
    symbolsMaxIndex = 0;
    nonterminalMaxIndex = 0;
    mapProds.clear();
}

bool Parser::hasError() {
    for(Issue &issue : issues)
        if(issue.type == Issue::Error)
            return true;
    return false;
}

void Parser::appendSymbol(Symbol::Type type, const QString &str) {
    mapSymbols.insert(Symbol(type, str), symbolsMaxIndex);
    symbolsMaxIndex++;
}



