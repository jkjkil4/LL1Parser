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

    /*  用于对涉及到的文件进行标记 */
    class Files
    {
    public:
        int filePathIndex(const CanonicalFilePath &cFilePath) const { return mFiles.value(cFilePath, -1); }
        CanonicalFilePath indexFilePath(int index) const { return mFiles.key(index); }
        bool contains(const CanonicalFilePath &cFilePath) const { return mFiles.contains(cFilePath); }
        int appendFilePath(const CanonicalFilePath &cFilePath) {
            if(!mFiles.contains(cFilePath)) {
                mFiles[cFilePath] = currentIndex;
                currentIndex++;
            }
            return currentIndex - 1;
        }

    private:
        int currentIndex = 0;
        QMap<CanonicalFilePath, int> mFiles;  //涉及的文件列表
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
    typedef QMap<QString, MapDivideds> FilesDivideds;   //key是文件名

    struct FileSymbol { int fileId; QString str; };

    // 用于存储总结果
    class TotalResult
    {
    public:
        friend class Parser_;

        const Files& files() const { return mFiles; }       //返回所有涉及的文件
        const Issues& issues() const { return mIssues; }    //返回所有问题
        const QList<FileSymbol>& symbolsToStr() const { return mSymbolsToStr; }         //返回id至名称的符号列表
        const QList<QMap<QString, int>>& symbolsToId() const { return mSymbolsToId; }   //返回名称至id的符号列表

    private:
        Files mFiles;   //所有涉及的文件
        Issues mIssues; //所有问题

        QList<FileSymbol> mSymbolsToStr;        //符号列表( id -> { fileId, str } )
        QList<QMap<QString, int>> mSymbolsToId; //符号列表( fileId -> { str -> id } )
    };


    Parser_(const QString &filePath);
    const TotalResult& result() const { return mResult; }

signals:
    void beforeReadFile(const QString &filePath);
    
private:
    bool divideFile(FilesDivideds &fd, const CanonicalFilePath &cFilePath); //用于分割文档 返回值若为true则完成了分割，否则为无法读取或已分割过
    void divideAndImportFile(FilesDivideds &fd, const CanonicalFilePath &cFilePath);    //分割并递归分割

    bool checkDividedArg(const QString &tag, const Divideds &divideds, const QString &fileName = ""); //判断Divideds是否含有有参数的Divided，若有，则产生警告

    TotalResult mResult;
};


