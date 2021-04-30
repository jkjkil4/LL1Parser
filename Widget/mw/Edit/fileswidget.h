#pragma once

#include <QListWidget>
#include <QFileInfo>
//#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

#include "header.h"

class FilesWidget : public QListWidget
{
public:
    explicit FilesWidget(QWidget *parent = nullptr);
    explicit FilesWidget(const QStringList &list, QWidget *parent = nullptr);
};