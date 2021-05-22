#pragma once

#include <Lib/recentfilemanager.h>
#include <Lib/fontfamily.h>

//#define DEBUG_DISABLEALL
#define C_BOTTOM QColor(230, 230, 230)
#define SUFFIX "lpp"

#define IsLetter(ch) ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
#define IsDigit(ch) (ch >= '0' && ch <= '9')

template<typename Fn>
inline int SearchText(const QString &text, int start, int end, Fn fn) {
    for(int i = start; i < end; i++) {
        if(fn(text[i])) 
            return i;
    }
    return -1;
}
template<typename Fn>
inline int SearchTextReverse(const QString &text, int start, int end, Fn fn) {
    for(int i = end - 1; i >= start; i--) {
        if(fn(text[i]))
            return i;
    }
    return -1;
}
inline bool SearchSpcFn(QChar ch) { return ch == '\t' || ch == ' '; }
inline bool SearchNonspcFn(QChar ch) { return ch != '\t' && ch != ' '; }

enum class UserRole : int {
    NoRole,
    ShowPlainText,          //显示纯文本    (QString windowTitle, QString text)
    ShowHtmlText,           //显示html    (QString windowTitle, QString htmlText)
    MoveDocumentCursor,     //移动文档鼠标位置    (QPoint(x:phrase, y:row))
    OpenFolder              //打开文件夹    (QString path)
};

// 用于统一化路径
class CanonicalFilePath
{
public:
    CanonicalFilePath() = default;
    CanonicalFilePath(const QString &filePath) 
        : mText(QDir(filePath).absolutePath()) {}
    const QString& text() { return mText; }
    inline operator const QString&() const { return mText; }
    inline bool operator<(const CanonicalFilePath &other) const { return mText < other.mText; }
    inline bool operator==(const CanonicalFilePath &other) const { return mText == other.mText; }
    inline bool operator==(const QString &s) const { return mText == CanonicalFilePath(s).mText; }
    friend inline bool operator==(const QString &s, const CanonicalFilePath &c) 
        { return CanonicalFilePath(s).mText == c.mText; }

private:
    QString mText;
};

extern RecentFileManager rfManager;
extern FontFamily fontSourceCodePro;
