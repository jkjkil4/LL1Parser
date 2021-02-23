#pragma once

#include <QTextDocument>
#include <QTextBlock>
#include <QApplication>
#include <QStyle>
#include <QRegularExpression>

#include <QDebug>

#include <Lib/header.h>


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

    struct Divided
    {
        struct Part
        {
            int row;
            QString text;

            friend inline QDebug& operator<<(QDebug &de, const Part &part) {
                de << QString::number(part.row) + "  " + part.text;
                return de;
            }
        };

        QList<Part> parts;

        friend inline QDebug& operator<<(QDebug &de, const Divided &divided) {
            de << "Divided(\n";
            for(const Part &part : divided.parts) {
                de << "    " << part << "\n";
            }
            de << ")";
            return de;
        }
    };

    static void divide(QTextDocument *doc);

    static void parse(QTextDocument *doc);

    static void clear();
    static QList<Error> errs;
    static QMap<QString, Divided> mapDivided;
};




