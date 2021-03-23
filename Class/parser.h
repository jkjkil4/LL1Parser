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
        enum Type {     //问题类型，Warning为警告，Error为错误
            Warning = QStyle::SP_MessageBoxWarning,
            Error = QStyle::SP_MessageBoxCritical
        } type;
        QString what;   //问题描述
        int row = -1;       //问题所在行数
        int phrase = -1;    //问题在行中的词数
        QList<QVariant> userDataList;   //自定义数据列表

        Issue(Type type, const QString &what, int row = -1, int phrase = -1, const QList<QVariant> &userDataList = QList<QVariant>())
            : type(type), what(what), row(row), phrase(phrase), userDataList(userDataList) {}

        QIcon icon() {  //返回对应的图标
            return QApplication::style()->standardIcon((QStyle::StandardPixmap)type);
        }

        friend inline QDebug& operator<<(QDebug &de, const Issue &err) {    //用于qDebug调试
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

    struct Divided  //分割后的文档(不同参数算作不同的Divided)
    {
        struct Part     //单独一行
        {
            int row;        //该行在原文档中的行数
            QString text;   //该行文字

            friend inline QDebug& operator<<(QDebug &de, const Part &part) {    //用于qDebug调试
                de << QString::number(part.row) + "  " + part.text;
                return de;
            }
        };

        QString arg;        //该分隔部分的参数
        QVector<int> rows;  //该分隔部分所有标记所在原文档的行数
        QList<Part> parts;  //该分隔部分的所有行

        friend inline QDebug& operator<<(QDebug &de, const Divided &divided) {  //用于qDebug调试
            de << "Divided(\n";
            for(const Part &part : divided.parts) {
                de << "    " << part << "\n";
            }
            de << ")";
            return de;
        }
    };
    struct Divideds //标记相同但参数不同的所有Divided
    {
        QList<Divided> listDivided;
        bool parsed = false;

        const Divided& value(const QString &arg, const Divided &def = Divided()) const {    //通过参数得到对应的Divided
            for(const Divided &divided : listDivided)
                if(divided.arg == arg)
                    return divided;
            return def;
        }

        /*
            重载[]运算符，目的是方便操作
            如果没有Divided与arg对应，那么就创建一个
            对于const来说，若没有Divided与arg对应，那么返回默认值
        */
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
        enum Type { Unknown, Terminal, Nonterminal } type = Unknown;    //符号类型
        QString str;    //符号文字

        Symbol() = default;
        Symbol(const QString &str) : str(str) {}
        Symbol(Type type, const QString &str) : type(type), str(str) {}

        inline bool operator==(const Symbol &other) const { return str == other.str; }
        inline bool operator<(const Symbol &other) const { return str < other.str; }
    };

    //产生式相关
    typedef QVector<int> SymbolVec;     //符号列表
    typedef QList<SymbolVec> Prods;     //某个非终结符可以推导出的所有产生式
    typedef QMap<int, Prods> ProdsMap;  //将非终结符和Prods相对应

    //SELECT集
    struct SelectSet { SymbolVec symbols, prod; };
    typedef QVector<SelectSet> SelectSets;

    static void divide(QTextDocument *doc);     //分隔文档
    static bool checkDividedArg(const QString &name, const Divideds &divideds); //判断Divideds是否含有有参数的Divided，若有，则产生警告
    static int findTrueRowByDividedRow(const Divideds &divideds, int dividedRow);   //以在分隔的部分中的行数找到对应原文档中的行数

    static void parse(QTextDocument *doc);  //分析

    static void parseTerminal(const QString &tag, const Divideds &divideds);    //处理终结符
    static void parseNonterminal(const QString &tag, const Divideds &divideds); //处理非终结符
    static void parseProduction(const QString &tag, const Divideds &divideds);  //处理产生式
    static void parseJs(const QString &tag, const Divideds &divideds);          //处理js脚本
    static void parseOutput(const QString &, const Divideds &divideds);      //处理输出

    static void parseNil();         //处理能否推导为空串
    static void parseFirstSet();    //处理FIRST集
    static void parseFollowSet();   //处理FOLLOW集
    static void parseSelectSet();   //处理SELECT集

    static void clear();        //清空内容
    static bool hasError();     //返回是否有错误
    static bool hasOutputFile() { return !listOutput.isEmpty(); }   //返回是否有要输出的文件
    static void appendSymbol(Symbol::Type type, const QString &str);    //添加符号
    static bool isNonterminal(int digit) { return digit >= 0 && digit <= nonterminalMaxIndex; }     //返回是否是非终结符
    static bool isTerminal(int digit) { return digit > nonterminalMaxIndex && digit <= terminalMaxIndex; }  //返回是否是终结符

    static QString outputDir(const QString &projPath, const QString &projName);     //返回要输出到的文件夹
    static void outputFile(const QString &projPath, const QString &projName);       //输出文件

    static QString formatProdsMap();    //返回 所有的产生式格式化为字符串的结果
    static QString formatNilVec();      //返回 空串情况格式化为字符串的结果
    static QString formatSet(const QVector<SymbolVec> &vecSet, bool useHtml, bool showNil);     //用于以下两个format
    static QString formatFirstSet(bool useHtml) { return formatSet(vecFirstSet, useHtml, true); }       //返回 所有FIRST集格式化为字符串的结果
    static QString formatFollowSet(bool useHtml) { return formatSet(vecFollowSet, useHtml, false); }    //返回 所有FOLLOW集格式化为字符串的结果
    static QString formatSelectSet(bool useHtml, QVector<SymbolVec> *pVecIntersectedSymbols = nullptr); //返回 所有SELECT集格式化为字符串的结果

    static QList<Issue> issues; //问题列表
    static bool hasProd;        //是否有产生式

    static QMap<QString, Divideds> mapDivideds;     //已分割的部分

    static QMap<Symbol, int> mapSymbols;    //用于将 字符串符号 和 数字符号 相对应的map
    static int symbolsMaxIndex;
    static int nonterminalMaxIndex;
    static int terminalMaxIndex;

    static ProdsMap mapProds;       //所有产生式
    static QVector<bool> vecNil;    //空串情况

    static QVector<SymbolVec> vecFirstSet;      //所有FIRST集
    static QVector<SymbolVec> vecFollowSet;     //所有FOLLOW集
    static QVector<SelectSets> vecSelectSets;   //所有SELECT集

    struct JS {     //弄结构体是为了方便动态创建
        QJSEngine engine;
        JSObject object;
    };
    static JS *js;  //用指针是为了方便动态创建以达到清空QJSEngine内容的目的
    static QString jsDebugMessage;  //js输出的调试信息

    struct Output { //输出的文件
        QString fileName;   //文件名
        QString text;       //文字
        friend inline QDebug& operator<<(QDebug &de, const Output &output) {    //用于qDebug调试
            de << "Output{" << "FileName:" << output.fileName << "," << "Text:" << output.text << "}";
            return de;
        }
    };
    static QList<Output> listOutput;    //所有要输出的文件
    
};




