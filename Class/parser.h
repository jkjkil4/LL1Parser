#pragma once

#include <QTextDocument>
#include <QTextBlock>
#include <QApplication>
#include <QStyle>

#include <QDebug>

#include <Lib/header.h>

/*
我对编译原理的学习不是很深
所以这部分代码可能很乱、很差
*/
class Parser : QObject
{
    Q_OBJECT
public:
    struct Error
    {
        Error(const QString &what, int row = -1, int col = -1) : what(what), row(row), phrase(col) {}
        QString what;
        int row = -1;
        int phrase = -1;

        friend inline QDebug& operator<<(QDebug &de, const Error &err) {
            de << "Error(" << err.what;
            if(err.row != -1) {
                de << "," << err.row;
                if(err.phrase != -1) {
                    de << "," << err.phrase;
                }
            }
            de << ")";
            return de;
        }
    };

    static void parse(QTextDocument *doc);

    static QList<Error> errs;
};

