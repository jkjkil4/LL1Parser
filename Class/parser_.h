#pragma once

#include <QTextDocument>
#include <QTextBlock>
#include <QApplication>
#include <QStyle>
#include <QRegularExpression>
#include <QJSEngine>
#include <QFileInfo>

#include "header.h"


/*  用于进行LL1语法分析
*/
class Parser : QObject
{
    Q_OBJECT
public:
    typedef QRegularExpression QRegex;

    /*  用于对涉及到的文件进行标记
    */
    class Files
    {
    public:
        static QString toCanonical(const QString &filePath) { return QFileInfo(filePath).canonicalFilePath(); }

        int filePathIndex(const QString &filePath) const { return mFiles.value(filePath, -1); }
        QString indexFilePath(int index) const { return mFiles.key(index, ""); }
        void appendFilePath(const QString &filePath) {
            if(!mFiles.contains(filePath)) {
                mFiles[filePath] = currentIndex;
                currentIndex++;
            }
        }

    private:
        int currentIndex = 0;
        QMap<QString, int> mFiles;  //涉及的文件列表
    };

    /*  描述在分析过程中遇到的问题
        分为 Warning 和 Error 两种
    */
    class Issue
    {
    public:
        enum Type {
            Warning = QStyle::SP_MessageBoxWarning,
            Error = QStyle::SP_MessageBoxCritical
        } type;         //类型
        QString what;   //描述
        int row = -1;   //行
        int col = -1;   //列
        
        Issue(Type type, const QString &what, int row = -1, int col = -1)
            : type(type), what(what), row(row), col(col) {}
        
        QIcon icon() const { return QApplication::style()->standardIcon((QStyle::StandardPixmap)type); }
    };

    /*  对QList<Issue>的封装
        用于更方便判断是否有错误
    */
    class Issues
    {
    public:
        void appendIssue(const Issue &issue) {  //添加问题
            if(issue.type == Issue::Error)
                mHasError = true;
            mList << issue;
        }
        bool hasError() const { return mHasError; }   //返回是否有错误
        const QList<Issue>& list() const { return mList; }    //返回问题列表
    private:
        QList<Issue> mList;     //问题列表
        bool mHasError = false; //是否有错误
    };

    /*  用于存储总结果
    */
    class TotalResult
    {
    public:
        const Files& files() const { return mFiles; }       //返回所有涉及的文件
        const Issues& issues() const { return mIssues; }    //返回所有问题

    private:
        Files mFiles;   //所有涉及的文件
        Issues mIssues; //所有问题
    };
};


