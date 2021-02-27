#pragma once

#include <QTextDocument>
#include <QTextBlock>
#include <QApplication>
#include <QStyle>
#include <QRegularExpression>

#include <QDebug>

#include <Lib/header.h>
#include "header.h"


class Parser : QObject
{
    Q_OBJECT
public:
    struct Issue    //问题
    {
        enum Type {
            Warning = QStyle::SP_MessageBoxWarning,
            Error = QStyle::SP_MessageBoxCritical
        } type;
        QString what;
        int row = -1;
        int phrase = -1;
        QList<QVariant> userDataList;

        Issue(Type type, const QString &what, int row = -1, int phrase = -1, const QList<QVariant> &userDataList = QList<QVariant>())
            : type(type), what(what), row(row), phrase(phrase), userDataList(userDataList) {}

        QIcon icon() {
            return QApplication::style()->standardIcon((QStyle::StandardPixmap)type);
        }

        friend inline QDebug& operator<<(QDebug &de, const Issue &err) {
            de << QString("Issue-") + (err.type == Warning ? "Warning" : "Error") + "(" << err.what;
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

    struct Divided  //分割后的文档
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

        QVector<int> rows;
        QList<Part> parts;
        bool parsed = false;

        friend inline QDebug& operator<<(QDebug &de, const Divided &divided) {
            de << "Divided(\n";
            for(const Part &part : divided.parts) {
                de << "    " << part << "\n";
            }
            de << ")";
            return de;
        }
    };

    struct Symbol   //符号
    {
        enum Type { Unknown, Terminal, Nonterminal } type = Unknown;
        QString str;

        Symbol() = default;
        Symbol(const QString &str) : str(str) {}
        Symbol(Type type, const QString &str) : type(type), str(str) {}

        inline bool operator==(const Symbol &other) const { return str == other.str; }
        inline bool operator<(const Symbol &other) const { return str < other.str; }
    };

    //产生式相关
    typedef QVector<int> SymbolVec;
    typedef QList<SymbolVec> Prods;
    typedef QMap<int, Prods> ProdsMap;

    //SELECT集
    struct SelectSet { SymbolVec symbols, prod; };
    typedef QVector<SelectSet> SelectSets;

    static void divide(QTextDocument *doc);

    static void parse(QTextDocument *doc);

    static void parseTerminal(const Divided &divided);      //处理终结符
    static void parseNonterminal(const Divided &divided);   //处理非终结符
    static void parseProduction(const Divided &divided);    //处理产生式

    static void parseNil();         //处理能否推导为空串
    static void parseFirstSet();    //处理FIRST集
    static void parseFollowSet();   //处理FOLLOW集
    static void parseSelectSet();   //处理SELECT集

    static void clear();
    static bool hasError();
    static void appendSymbol(Symbol::Type type, const QString &str);
    static bool isNonterminal(int digit) { return digit >= 0 && digit <= nonterminalMaxIndex; }
    static bool isTerminal(int digit) { return digit > nonterminalMaxIndex && digit <= terminalMaxIndex; }

    static QString formatProdsMap();
    static QString formatNilVec();
    static QString formatSet(const QVector<SymbolVec> &vecSet, bool useHtml, bool showNil);
    static QString formatFirstSet(bool useHtml) { return formatSet(vecFirstSet, useHtml, true); }
    static QString formatFollowSet(bool useHtml) { return formatSet(vecFollowSet, useHtml, false); }
    static QString formatSelectSet(bool useHtml, QVector<SymbolVec> *pVecIntersectedSymbols = nullptr);

    static QList<Issue> issues;

    static QMap<QString, Divided> mapDivided;

    static QMap<Symbol, int> mapSymbols;
    static int symbolsMaxIndex;
    static int nonterminalMaxIndex;
    static int terminalMaxIndex;

    static ProdsMap mapProds;
    static QVector<bool> vecNil;

    static QVector<SymbolVec> vecFirstSet;
    static QVector<SymbolVec> vecFollowSet;
    static QVector<SelectSets> vecSelectSets;
};




