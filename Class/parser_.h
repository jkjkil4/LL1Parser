#pragma once

#include <QApplication>
#include <QStyle>
#include <QRegularExpression>
#include <QJSEngine>
#include <QFileInfo>

#include "header.h"
#include <Lib/header.h>


/*  用于进行LL1语法分析
*/
class Parser_ : QObject
{
    Q_OBJECT
public:
    typedef QRegularExpression QRegex;

    //用于统一化路径
    class CanonicalFilePath
    {
    public:
        CanonicalFilePath() = default;
        CanonicalFilePath(const QString &filePath) 
            : mText(QDir(filePath).absolutePath()) {}
        const QString& text() { return mText; }
        inline operator const QString&() const { return mText; }
        inline bool operator<(const CanonicalFilePath &other) const { return mText < other.mText; }

    private:
        QString mText;
    };

    //用于创建对应关系
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
            mMap[key] = currentIndex;
            currentIndex++;
            return currentIndex - 1;
        }

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
        QString fileName;   //文件名称
        int row = -1;   //行
        int col = -1;   //列
        
        Issue(Type type, const QString &what, const QString &fileName = "", int row = -1, int col = -1)
            : type(type), what(what), fileName(fileName), row(row), col(col) {}
        
        QIcon icon() const { return QApplication::style()->standardIcon((QStyle::StandardPixmap)type); }
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

    //分割后的文档(不同参数算作不同的Divided)
    struct Divided
    {
        struct Part     //单独一行
        {
            int row;        //该行在原文档中的行数
            int col;        //该行在原文档中的列数的偏移
            QString text;   //该行文字
        };

        // QString arg;        //该分割部分的参数
        QVector<int> rows;  //该分割部分所有标记所在原文档的行数
        QList<Part> parts;  //该分割部分的所有行
    };

    //标记相同但参数不同的所有Divided
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

    //声明位置
    struct DeclarePos { int row, col; };

    //同时具有文件id和符号id
    struct FileSymbol
    { 
        int fileId, symbolId;
        inline bool operator==(const FileSymbol &other) const { return fileId == other.fileId && symbolId == other.symbolId; }
        inline bool operator<(const FileSymbol &other) const {
            if(fileId != other.fileId)
                return fileId < other.fileId;
            return symbolId < other.symbolId;
        }
    };

    // 用于存储总结果
    class TotalResult
    {
    public:
        friend class Parser_;

        const ValueMap<CanonicalFilePath>& files() const { return mFiles; } //返回所有涉及的文件
        const ValueMap<QString>& symbols() const { return mSymbols; }       //返回所有符号
        const Issues& issues() const { return mIssues; }    //返回所有问题

    private:
        ValueMap<CanonicalFilePath> mFiles;   //所有涉及的文件
        Issues mIssues;     //所有问题

        ValueMap<QString> mSymbols;     //所有符号
        QMap<FileSymbol, DeclarePos> mSymbolsDeclarePos;    //记录符号声明位置
        QMap<FileSymbol, bool> mSymbolsUsed;    //标记符号是否使用过
    };

    Parser_(const QString &filePath);
    const TotalResult& result() const { return mResult; }

signals:
    void beforeReadFile(const QString &filePath);
    
private:
    bool divideFile(FilesDivideds &fd, const CanonicalFilePath &cFilePath); //用于分割文档 返回值若为true则完成了分割，否则为无法读取或已分割过
    void divideAndImportFile(FilesDivideds &fd, const CanonicalFilePath &cFilePath);    //分割并递归分割
    bool checkDividedArg(const QString &tag, const Divideds &divideds, const QString &fileName = ""); //判断Divideds是否含有有参数的Divided，若有，则产生警告

    typedef void(Parser_::*ParseFn)(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds);
    void parseSymbol(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds);
    // void parseNonterminal(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds);
    // void parseTerminal(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds);

    TotalResult mResult;
};


