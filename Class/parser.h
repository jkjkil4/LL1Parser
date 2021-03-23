#pragma once

#include <QTextDocument>
#include <QTextBlock>
#include <QApplication>
#include <QStyle>
#include <QRegularExpression>
#include <QJSEngine>

#include <QDebug>

#include "jsobject.h"

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

        QString arg;
        QVector<int> rows;
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
    struct Divideds
    {
        QList<Divided> listDivided;
        bool parsed = false;

        const Divided& value(const QString &arg, const Divided &def = Divided()) const {
            for(const Divided &divided : listDivided)
                if(divided.arg == arg)
                    return divided;
            return def;
        }
        Divided& operator[](const QString &arg) {
            for(Divided &divided : listDivided)
                if(divided.arg == arg)
                    return divided;
            listDivided << Divided();
            Divided &res = *listDivided.rbegin();
            res.arg = arg;
            return res;
        }
        const Divided& operator[](const QString &arg) const { return value(arg); }
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
    static bool checkDividedArg(const QString &name, const Divideds &divideds);
    static int findTrueRowByDividedRow(const Divideds &divideds, int dividedRow);

    static void parse(QTextDocument *doc);

    static void parseTerminal(const QString &tag, const Divideds &divideds);    //处理终结符
    static void parseNonterminal(const QString &tag, const Divideds &divideds); //处理非终结符
    static void parseProduction(const QString &tag, const Divideds &divideds);  //处理产生式
    static void parseJs(const QString &tag, const Divideds &divideds);          //处理js脚本
    static void parseOutput(const QString &, const Divideds &divideds);      //处理输出

    static void parseNil();         //处理能否推导为空串
    static void parseFirstSet();    //处理FIRST集
    static void parseFollowSet();   //处理FOLLOW集
    static void parseSelectSet();   //处理SELECT集

    static void clear();
    static bool hasError();
    static bool hasOutputFile() { return !listOutput.isEmpty(); }
    static void appendSymbol(Symbol::Type type, const QString &str);
    static bool isNonterminal(int digit) { return digit >= 0 && digit <= nonterminalMaxIndex; }
    static bool isTerminal(int digit) { return digit > nonterminalMaxIndex && digit <= terminalMaxIndex; }

    static QString outputDir(const QString &projPath, const QString &projName);
    static void outputFile(const QString &projPath, const QString &projName);

    static QString formatProdsMap();
    static QString formatNilVec();
    static QString formatSet(const QVector<SymbolVec> &vecSet, bool useHtml, bool showNil);
    static QString formatFirstSet(bool useHtml) { return formatSet(vecFirstSet, useHtml, true); }
    static QString formatFollowSet(bool useHtml) { return formatSet(vecFollowSet, useHtml, false); }
    static QString formatSelectSet(bool useHtml, QVector<SymbolVec> *pVecIntersectedSymbols = nullptr);

    static QList<Issue> issues;
    static bool hasProd;

    static QMap<QString, Divideds> mapDivideds;

    static QMap<Symbol, int> mapSymbols;
    static int symbolsMaxIndex;
    static int nonterminalMaxIndex;
    static int terminalMaxIndex;

    static ProdsMap mapProds;
    static QVector<bool> vecNil;

    static QVector<SymbolVec> vecFirstSet;
    static QVector<SymbolVec> vecFollowSet;
    static QVector<SelectSets> vecSelectSets;

    struct JS {
        QJSEngine engine;
        JSObject object;
    };
    static JS *js;
    static QString jsDebugMessage;

    struct Output {
        QString fileName;
        QString text;
        friend inline QDebug& operator<<(QDebug &de, const Output &output) {
            de << "Output{" << "FileName:" << output.fileName << "," << "Text:" << output.text << "}";
            return de;
        }
    };
    static QList<Output> listOutput;
    
};




