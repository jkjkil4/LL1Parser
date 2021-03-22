#pragma once

#include <Lib/recentfilemanager.h>
#include <Lib/fontfamily.h>

#define C_BOTTOM QColor(230, 230, 230)

#define SUFFIX "lpp"

enum class UserRole : int {
    NoRole,
    ShowPlainText,          //显示纯文本    (QString windowTitle, QString text)
    ShowHtmlText,           //显示html    (QString windowTitle, QString htmlText)
    MoveDocumentCursor,     //移动文档鼠标位置    (QPoint(x:phrase, y:row))
    OpenFolder              //打开文件夹    (QString path)
};

extern RecentFileManager rfManager;
extern FontFamily fontSourceCodePro;
