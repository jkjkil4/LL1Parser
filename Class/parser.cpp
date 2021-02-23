#include "parser.h"

QList<Parser::Error> Parser::errs;
QMap<QString, Parser::Divided> Parser::mapDivided;

void Parser::divide(QTextDocument *doc) {
    QRegularExpression regExp("%\\[(.*?)\\]%");     //正则表达式
    int count = doc->lineCount();

    Divided *divided = &mapDivided[""];     //默认Divided
    repeat(int, i, count) {     //遍历文档的所有行
        QString line = doc->findBlockByLineNumber(i).text();    //当前行
        int start = 0;
        QRegularExpressionMatch match = regExp.match(line);     //使用正则表达式获取内容
        while(match.hasMatch()) {   //重复直到正则表达式没获取到内容
            QString mid = line.mid(start, match.capturedStart() - start);
            divided->parts.append(Divided::Part{ i, mid });

            start = match.capturedEnd();
            divided = &mapDivided[match.captured(1)];
            match = regExp.match(line, match.capturedEnd());
        }
        QString right = line.right(line.length() - start);
        divided->parts.append(Divided::Part{ i, right });
    }
}

void Parser::parse(QTextDocument *doc) {
    clear();
    divide(doc);
    qDebug().noquote() << mapDivided;

    int count = doc->lineCount();
    repeat(int, i, count) {
        QString line = doc->findBlockByLineNumber(i).text().simplified().trimmed();
        QStringList list = line.split(' ', QString::SkipEmptyParts);
        int size = list.size();
        if(size < 2) {
            if(size == 1)
                errs.append(Error(tr("Lack of \"->\" to represent production"), i));
            continue;
        }
        if(list[1] != "->") {
            errs.append(Error(tr("Should be \"->\" instead of \"%1\"").arg(list[1]), i, 1));
            continue;
        }
    }
}

void Parser::clear() {
    errs.clear();
    mapDivided.clear();
}


