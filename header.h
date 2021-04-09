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

extern RecentFileManager rfManager;
extern FontFamily fontSourceCodePro;
