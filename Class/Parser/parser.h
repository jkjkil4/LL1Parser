#pragma once

#include <QApplication>
#include <QStyle>
#include <QRegularExpression>
#include <QJSEngine>
#include <QFileInfo>

#include "header.h"
#include "js.h"
#include "jsobject.h"
#include "Widget/Dialog/delayedverifydialog.h"
#include "Class/ProjListWidgetItem/projlistwidgetitem.h"
#include <Lib/header.h>
#include <Lib/paint.h>


// 用于进行LL1语法分析
class Parser : QObject
{
    Q_OBJECT
public:
    typedef QRegularExpression QRegex;

    // 用于创建对应关系
    template<typename Key>
    class ValueMap 
    {
    public:
        int keyIndex(const Key &key) const { return mMap.value(key, -1); }
        Key indexKey(int index) const { return mMap.key(index); }
        bool contains(const Key &key) const { return mMap.contains(key); }
        int appendKey(const Key &key) {
            if(mMap.contains(key))
                return keyIndex(key);
            int size = mMap.size();
            mMap[key] = size;
            return size;
        }
        const QMap<Key, int> map() const { return mMap; }

    private:
        int currentIndex = 0;
        QMap<Key, int> mMap;
    };

    /*  描述在分析过程中遇到的问题
        分为 Warning 和 Error 两种 */
    class Issue
    {
    public:
        enum Type {
            Warning = QStyle::SP_MessageBoxWarning,
            Error = QStyle::SP_MessageBoxCritical
        } type;         //类型
        QString what;   //描述
        QString filePath;   //文件名称
        int row = -1;   //行
        int col = -1;   //列

        Issue(Type type, const QString &what, QString filePath = "") : type(type), what(what), filePath(filePath) {}
        Issue(Type type, const QString &what, const QString &filePath, int row, int col = -1)
            : type(type), what(what), filePath(filePath), row(row), col(col) 
        {
            item = new PLWI_MoveDocCursor(filePath, row, col);
        }
        Issue(Type type, const QString &what, ProjListWidgetItem *item)
            : type(type), what(what), item(item) {}

        void copyFrom(const Issue &other) {
            type = other.type;
            what = other.what;
            filePath = other.filePath;
            row = other.row;
            col = other.col;
            item = other.newItem();
        }
        Issue(const Issue &other) { copyFrom(other); }
        Issue& operator=(const Issue &other) { copyFrom(other); return *this; }

        ~Issue() { j::SafeDelete(item); }
        
        ProjListWidgetItem* newItem() const { return item ? item->copy() : nullptr; }
        QIcon icon() const { return QApplication::style()->standardIcon((QStyle::StandardPixmap)type); }
    
    private:
        ProjListWidgetItem *item = nullptr;
    };

    /*  对QList<Issue>的封装
        用于更方便判断是否有错误 */
    class Issues
    {
    public:
        void append(const Issue &issue) {  //添加问题
            if(issue.type == Issue::Error)
                mHasError = true;
            mList << issue;
        }
        bool hasError() const { return mHasError; }   //返回是否有错误
        const QList<Issue>& list() const { return mList; }    //返回问题列表
        inline Issues& operator<<(const Issue &issue) { //重载"<<"运算符，调用append
            append(issue); 
            return *this; 
        }
    private:
        QList<Issue> mList;     //问题列表
        bool mHasError = false; //是否有错误
    };

    // 分割后的文档(不同参数算作不同的Divided)
    struct Divided
    {
        struct Part     //单独一行
        {
            int row;        //该行在原文档中的行数
            int col;        //该行在原文档中的列数的偏移
            QString text;   //该行文字
        };

        QVector<int> rows;  //该分割部分所有标记所在原文档的行数
        QList<Part> parts;  //该分割部分的所有行
    };

    // 标记相同但参数不同的所有Divided
    class Divideds
    {
    public:
        VAR_FUNC(parsed, setParsed, mParsed, bool, , )
    
        const QMap<QString, Divided>& map() const { return mMap; }
        Divided& operator[](const QString &arg) { return mMap[arg]; }
        
    private:
        QMap<QString, Divided> mMap;
        bool mParsed = false;
    };
    typedef QMap<QString, Divideds> MapDivideds;        //key是标记的名称
    typedef QMap<CanonicalFilePath, MapDivideds> FilesDivideds;   //key是文件路径

    // 声明位置
    struct DeclarePos { int row, col; };

    // 记录Import关系的key
    struct ImportKey 
    { 
        int fileId; QString abbre;
        inline bool operator<(const ImportKey &other) const {
            if(fileId != other.fileId)
                return fileId < other.fileId;
            return abbre < other.abbre;
        }
    };
    // 记录Import关系的Value
    struct ImportValue { int fileId; DeclarePos declarePos; };

    // 文件-元素
    struct FileElement
    {
        int fileId; QString str;
        QString format() { return fileId == 0 ? str : QString::number(fileId) + ':' + str; }
        inline bool operator<(const FileElement &other) const { 
            if(fileId != other.fileId)
                return fileId < other.fileId;
            return str < other.str;
        }
    };
    typedef FileElement FileSymbol, FileAction;
    // 符号信息
    struct SymbolInfo { DeclarePos declarePos; bool used; };
    // 语义动作信息
    struct ActionInfo { DeclarePos declarePos; QString val; bool used; };

    // 产生式相关
    typedef QVector<int> SymbolVec;         //符号列表
    struct ProdAction { int pos, id; };     //语义动作
    struct Prod     //产生式
    {
        SymbolVec symbols;
        QList<ProdAction> actions;
        DeclarePos declarePos;
        inline bool operator==(const Prod &other) { return symbols == other.symbols; }
    };
    typedef QList<Prod> Prods;
    typedef QMap<int, Prods> MapProds;

    // SELECT集
    struct SelectSet { SymbolVec symbols; const Prod &prod; };
    typedef QVector<SelectSet> SelectSets;

    // JSEngine和JSObject
    struct JS { JSEngine engine; JSObject obj; };
    struct JSDebugMessage { int fileId; QString text; };

    // 输出内容
    struct Output
    {
        QString basePath, path;
        QString text;
    };

    // 用于存储总结果
    class Result
    {
    public:
        friend class Parser;

        const ValueMap<CanonicalFilePath>& files() const { return mFiles; }
        const QMap<ImportKey, ImportValue>& fileRels() const { return mFileRels; }
        const Issues& issues() const { return mIssues; }

        const ValueMap<FileSymbol>& symbols() const { return mSymbols; }
        const QList<SymbolInfo>& symbolsInfo() const { return mSymbolsInfo; }
        int nonterminalMaxIndex() const { return mNonterminalMaxIndex; }
        int terminalMaxIndex() const { return mTerminalMaxIndex; }

        bool hasProd() const { return mHasProd; }
        const MapProds& prods() const { return mProds; }

        const QVector<bool>& symbolNil() const { return mSymbolsNil; }
        const QVector<SymbolVec>& firstSet() const { return mFirstSet; }
        const QVector<SymbolVec>& followSet() const { return mFollowSet; }
        const QVector<SelectSets>& selectSets() const { return mSelectSets; }

        QList<JSDebugMessage> jsDebugMessage() const;
        const QList<Output>& outputList() const { return mOutput; }

        bool isNonterminal(int id) { return id >= 0 && id <= mNonterminalMaxIndex; }
        bool isTerminal(int id) { return id > mNonterminalMaxIndex && id <= mTerminalMaxIndex; }

        QStringList output();

        QString formatFiles();      //将分析过的文件格式化为文本
        QString formatSymbolsNil(); //将空串情况格式化为文本
        QString formatSet(const QVector<SymbolVec> &vecSet, bool useHtml, bool showNil);    //将集合格式化为文本，用于下面两个函数
        QString formatFirstSet(bool useHtml) { return formatSet(mFirstSet, useHtml, true); }       //将FIRST集格式化为文本
        QString formatFollowSet(bool useHtml) { return formatSet(mFollowSet, useHtml, true); }     //将FOLLOW集格式化为文本
        QString formatSelectSet(bool useHtml, QVector<SymbolVec> *pVecIntersectedSymbols = nullptr);    //将SELECT集格式化为文本

    private:
        FilesDivideds mFilesDivideds;

        ValueMap<CanonicalFilePath> mFiles;     //所有涉及的文件
        QMap<ImportKey, ImportValue> mFileRels; //文件Import关系
        Issues mIssues;     //所有问题

        ValueMap<FileSymbol> mSymbols;      //所有符号
        QList<SymbolInfo> mSymbolsInfo;     //符号的信息
        int mNonterminalMaxIndex = -1;
        int mTerminalMaxIndex = -1;

        ValueMap<FileAction> mActions;     //所有语义动作
        QList<ActionInfo> mActionsInfo; //语义动作的信息

        bool mHasProd = false;
        MapProds mProds;      //所有产生式

        QVector<bool> mSymbolsNil;    //空串情况
        QVector<SymbolVec> mFirstSet;       //所有FIRST集
        QVector<SymbolVec> mFollowSet;      //所有FOLLOW集
        QVector<SelectSets> mSelectSets;    //所有SELECT集

        QMap<int, JS*> mJS;
        QList<Output> mOutput;
    };

    Parser(const QString &filePath, QWidget *dialogParent = nullptr, QObject *parent = nullptr);
    ~Parser() override;
    Result& result() { return mResult; }

private:
    bool divideFile(const CanonicalFilePath &cFilePath, const QString &basePath = ""); //用于分割文档 返回值若为true则完成了分割，否则为无法读取或已分割过
    void divideAndImportFile(const CanonicalFilePath &cFilePath, const QString &basePath = "");    //分割并递归分割
    bool checkDividedArg(const QString &tag, const Divideds &divideds, const QString &filePath = ""); //判断Divideds是否含有有参数的Divided，若有，则产生警告
    int findTrueRowByDividedRow(const Divideds &divideds, int dividedRow);   //在分隔的部分中的行数找到对应原文档中的行数
    FileElement transFileElement(int fileId, const QString &name);

    typedef void(Parser::*ParseFn)(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds);
    void parseSymbol(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds);
    void parseAction(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds);
    void parseProd(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds);
    void parseSymbolsNil();
    void parseFirstSet();
    void parseFollowSet();
    void parseSelectSets();
    void parseJS(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds);
    void parseOutput(const CanonicalFilePath &cFilePath, const QString &, const Divideds &divideds);

    QWidget *mDialogParent;
    Result mResult;
};


