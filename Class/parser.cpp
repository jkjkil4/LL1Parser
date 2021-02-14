#include "parser.h"

QList<Parser::Error> Parser::errs;

void Parser::parse(QTextDocument *doc) {
    errs.clear();

    int count = doc->lineCount();
    repeat(int, i, count) {
        QString line = doc->findBlockByLineNumber(i).text().simplified().trimmed();
        QStringList list = line.split(' ', QString::SkipEmptyParts);
        int size = list.size();
        if(size < 2) {
            if(size == 1)
                errs.append(Error(tr("Lack of \"->\" to represent production"), i + 1));
            continue;
        }
        if(list[1] != "->") {
            errs.append(Error(tr("Should be \"->\" instead of \"%1\"").arg(list[1]), i + 1, 2));
            continue;
        }
    }
}


