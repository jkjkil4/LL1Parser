#include "parser_.h"

#include <QDebug>

Parser_::Parser_(const QString &filePath) {
    FilesDivideds filesDivideds;
    CanonicalFilePath cFilePath(filePath);

    auto tryParse = [this, &filesDivideds](const QString &tag, ParseFn fn) {    //尝试分析指定标记
        for(auto fileIter = filesDivideds.begin(); fileIter != filesDivideds.end(); ++fileIter) {   //遍历所有文件
            const CanonicalFilePath &cFilePath = fileIter.key();   //文件名
            auto iter = fileIter->find(tag);    //查找标记
            if(iter != fileIter->end()) {
                (this->*fn)(cFilePath, tag, *iter);  //分析标记
                iter->setParsed(true);
            }
        }
    };

    divideAndImportFile(filesDivideds, cFilePath);
    int fileId = mResult.mFiles.keyIndex(cFilePath);

    if(mResult.mIssues.hasError())
        goto End;

    mResult.mSymbols.appendKey(FileSymbol{ fileId, "S" });  //开始符号
    mResult.mSymbolsInfo.append(SymbolInfo{ DeclarePos(), true });
    tryParse("Nonterminal", parseSymbol);   //分析非终结符
    mResult.mNonterminalMaxIndex = mResult.mSymbolsInfo.size() - 1;

    mResult.mSymbols.appendKey(FileSymbol{ fileId, "$" });  //结束符号
    mResult.mSymbolsInfo.append(SymbolInfo{ DeclarePos(), true });
    tryParse("Terminal", parseSymbol);      //分析终结符
    mResult.mTerminalMaxIndex = mResult.mSymbolsInfo.size() - 1;

    tryParse("Action", parseAction);
    tryParse("Production", parseProd);
    if(mResult.mHasProd) {
        //检查是否有未使用的符号
        int index = 0;
        for(const SymbolInfo &info : mResult.mSymbolsInfo) {
            if(!info.used) {
                FileSymbol fs = mResult.mSymbols.indexKey(index);
                mResult.mIssues << Issue(Issue::Warning, tr("Unused symbol \"%1\"").arg(fs.str),
                    mResult.mFiles.indexKey(fs.fileId), info.declarePos.row, info.declarePos.col);
            }
            index++;
        }
        //检查是否有未使用的语义动作
        index = 0;
        for(const ActionInfo &info : mResult.mActionsInfo) {
            if(!info.used) {
                FileAction fa = mResult.mActions.indexKey(index);
                mResult.mIssues << Issue(Issue::Warning, tr("Unused semantic action \"%1\"").arg(fa.str),
                    mResult.mFiles.indexKey(fa.fileId), info.declarePos.row, info.declarePos.col);
            }
        }
    } else mResult.mIssues << Issue(Issue::Error, tr("Cannot find any production"));
    
    if(mResult.mIssues.hasError())
        goto End;

    {//检查是否有未知标记
        QString trUnkTag = tr("Unknown tag \"%1\"");
        for(auto iter = filesDivideds.cbegin(); iter != filesDivideds.cend(); ++iter) {  //遍历所有文件
            const QString &fileName = QFileInfo(iter.key()).fileName();
            const MapDivideds &mapDivideds = iter.value();
            for(auto iter2 = mapDivideds.cbegin(); iter2 != mapDivideds.cend(); ++iter2) {  //遍历该文件的所有分割部分
                const QString &tag = iter2.key();
                const Divideds &divideds = iter2.value();
                if(!divideds.parsed()) {
                    for(const Divided &divided : divideds.map()) {
                        for(int row : divided.rows) {
                            mResult.mIssues << Issue(Issue::Warning, trUnkTag.arg(tag), fileName, row);
                        }
                    }
                }
            }
        }
    }
    
    End:
    for(auto iter = mResult.mProds.cbegin(); iter != mResult.mProds.cend(); ++iter) {
        for(const Prod &prod : iter.value()) {
            QDebug de = qDebug().noquote();
            de << mResult.mSymbols.indexKey(iter.key()).str;
            for(int symbol : prod.symbols) {
                de << "  " << mResult.mSymbols.indexKey(symbol).str;
            }           
        }
    }
    for(const Issue &issue : mResult.mIssues.list()) {
        qDebug().noquote() << (issue.type == Issue::Error ? "\033[31m" : "\033[33m")
            << (issue.type == Issue::Error ? "Error" : "Warning")
            << "\033[0m    "
            << issue.fileName
            << "  "
            << issue.row
            << "  "
            << issue.col
            << "  "
            << issue.what;
    }
}

bool Parser_::isNonterminal(int id) { return id >= 0 && id <= mResult.mNonterminalMaxIndex; }
bool Parser_::isTerminal(int id) { return id > mResult.mNonterminalMaxIndex && id <= mResult.mTerminalMaxIndex; }

bool Parser_::divideFile(FilesDivideds &fd, const CanonicalFilePath &cFilePath, const QString &basePath) {
    if(mResult.mFiles.contains(cFilePath))  //如果处理过该文件，则return
        return false;
    mResult.mFiles.appendKey(cFilePath);    //标记处理过该文件
    emit beforeReadFile(cFilePath);  //在读取文件之前发出信号

    QFile file(cFilePath);   //文件
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) { //尝试打开文件
        mResult.mIssues << Issue(Issue::Error, tr("Cannot open the file \"%1\"").arg(QFileInfo(cFilePath).fileName()), basePath);
        return false;
    }

    QTextStream in(&file);  //用于读取文件
    QRegularExpression regex("%\\[(.*?)(?:\\:(.*?)){0,1}\\]%");     //正则表达式
    MapDivideds &mapDivideds = fd[cFilePath];    //得到该文件对应的mapDivideds
    int row = 0;    //用于记录行数
    Divided *pDivided = nullptr;    //分割部分
    while(!in.atEnd()) {    //循环遍历文件的每一行
        QString line = in.readLine();   //当前行
        int start = 0;  //用于记录偏移
        QRegularExpressionMatch match = regex.match(line);  //使用正则表达式获取内容
        while(match.hasMatch()) {   //重复直到正则表达式获取不到内容
            QString mid = line.mid(start, match.capturedStart() - start);   //得到获取的部分
            if(pDivided && !mid.isEmpty())  //如果不为空，则添加到分割部分中，如此判断是为了跳过标记前后的空行
                pDivided->parts << Divided::Part{ row, start, mid };

            start = match.capturedEnd();    //根据获取的结尾设置偏移
            pDivided = &mapDivideds[match.captured(1)][match.captured(2)];  //重新设置分割部分
            pDivided->rows << row;  //向分割部分中添加该标记的行数
            match = regex.match(line, start);   //从偏移处继续获取内容
        }
        QString right = line.right(line.length() - start);  //得到结尾剩余部分(如果该行没有标记则是整行)
        if(pDivided && (!right.isEmpty() || start == 0))    //如果 不为空 或者 为一整行，则添加到分割部分中
            pDivided->parts.append(Divided::Part{ row, start, right });

        row++;
    }

    file.close();   //结束对文件的操作

    return true;
}
void Parser_::divideAndImportFile(FilesDivideds &fd, const CanonicalFilePath &cFilePath, const QString &basePath) {
    if(!divideFile(fd, cFilePath, basePath))    //尝试分割，若返回值为false则return
        return;

    int fileId = mResult.mFiles.keyIndex(cFilePath);
    MapDivideds &md = fd[cFilePath];   //得到分割的内容
    auto iter = md.find("Import");     //查找"Import"标记
    if(iter == md.end())   //若没有，则return
        return;
    
    Divideds &imports = *iter;    // 导入文件 的列表
    imports.setParsed(true);
    checkDividedArg("Import", imports, QFileInfo(cFilePath).fileName());
    for(const Divided &divided : imports.map()) {
        for(const Divided::Part &part : divided.parts) {  //遍历所有行
            int len = part.text.length();

            //获取缩写部分
            int abbreStart = SearchText(part.text, 0, len, SearchNonspcFn);
            if(abbreStart == -1)
                continue;
            int abbreEnd = SearchText(part.text, abbreStart + 1, len, SearchSpcFn);
            int trueAbbreEnd = (abbreEnd == -1 ? part.text.length() : abbreEnd);
            QString abbre = part.text.mid(abbreStart, trueAbbreEnd - abbreStart);
            
            //获取路径部分
            QString importFilePath;
            int filePathStart = SearchText(part.text, trueAbbreEnd + 1, len, SearchNonspcFn);
            if(filePathStart != -1) {
                int filePathEnd = SearchTextReverse(part.text, filePathStart, len, SearchNonspcFn) + 1;
                importFilePath = part.text.mid(filePathStart, filePathEnd - filePathStart);
            }
            
            //如果路径为空，则报错
            if(importFilePath.isEmpty()) {
                mResult.mIssues << Issue(Issue::Error, tr("The import path of \"%1\" is empty").arg(abbre),
                    cFilePath, part.row, part.col);
                continue;
            }

            //递归分割文件
            QString aImportFilePath = QDir(QFileInfo(cFilePath).path()).absoluteFilePath(importFilePath);
            divideAndImportFile(fd, aImportFilePath, cFilePath);
            ImportKey key{ fileId, abbre };
            auto iterRel = mResult.mFileRels.constFind(key);
            if(iterRel != mResult.mFileRels.cend()) {   //检查是否重复
                mResult.mIssues << Issue(Issue::Error, tr("Redefintition of \"%1\" (First definition is at row %2, col %3)")
                    .arg(abbre, QString::number(iterRel->declarePos.row + 1), QString::number(iterRel->declarePos.col + 1)),
                    cFilePath, part.row, part.col);
            }
            mResult.mFileRels[key] = { mResult.mFiles.keyIndex(aImportFilePath), DeclarePos{ part.row, part.col } };
        }
    }
}
bool Parser_::checkDividedArg(const QString &tag, const Divideds &divideds, const QString &filePath) {
    bool hasArg = false;
    const QMap<QString, Divided>& map = divideds.map();
    for(auto iter = map.cbegin(); iter != map.cend(); ++iter) {   //遍历所有的Divided
        if(iter.key() != "") {  //如果有参数
            hasArg = true;
            QString str = tr("The tag \"%1\" does not need to provide any parameter").arg(tag); //警告信息
            for(int row : iter.value().rows)    //遍历标记的所有行
                mResult.mIssues << Issue(Issue::Warning, str, filePath, row);
        }
    }
    return hasArg;
}

void Parser_::parseSymbol(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds) {
    int fileId = mResult.mFiles.keyIndex(cFilePath);
    checkDividedArg(tag, divideds, cFilePath);
    for(const Divided &divided : divideds.map()) {
        for(const Divided::Part &part : divided.parts) {    //遍历所有行
            int len = part.text.length();
            int symbolEnd = 0;
            for(int symbolStart = SearchText(part.text, 0, len, SearchNonspcFn); 
                symbolStart != -1; 
                symbolStart = SearchText(part.text, symbolEnd + 1, len, SearchNonspcFn)) 
            {
                symbolEnd = SearchText(part.text, symbolStart + 1, len, SearchSpcFn);
                if(symbolEnd == -1)
                    symbolEnd = len;
                QString symbolStr = part.text.mid(symbolStart, symbolEnd - symbolStart);    //符号名称
                
                if(symbolStr == 'S' || symbolStr == '$') {  //检查是否和自带符号冲突
                    mResult.mIssues << Issue(Issue::Error, tr("Cannot use \"%1\" as symbol").arg(symbolStr), 
                        cFilePath, part.row, part.col + symbolStart);
                    continue;
                }
                if(symbolStr.contains(':')) {   //检查是否包含":"
                    mResult.mIssues << Issue(Issue::Error, tr("Symbol name \"%1\" cannot contain \":\"").arg(symbolStr), 
                        cFilePath, part.row, part.col + symbolStart);
                    continue;
                }

                FileSymbol fileSymbol{ fileId, symbolStr };
                //检查是否声明过
                if(mResult.mSymbols.contains(fileSymbol)) {
                    const DeclarePos &dp = mResult.mSymbolsInfo[mResult.mSymbols.keyIndex(fileSymbol)].declarePos;
                    mResult.mIssues << Issue(Issue::Warning, tr("Redefintition of \"%1\" (First definition is at row %2, col %3)")
                        .arg(symbolStr, QString::number(dp.row + 1), QString::number(dp.col + 1)),
                        cFilePath, part.row, part.col + symbolStart);
                    continue;
                }
                //记录该符号
                mResult.mSymbols.appendKey(fileSymbol);
                mResult.mSymbolsInfo << SymbolInfo{ DeclarePos{ part.row, part.col + symbolStart }, false };
            }
        }
    }
}
void Parser_::parseAction(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds) {
    int fileId = mResult.mFiles.keyIndex(cFilePath);
    checkDividedArg(tag, divideds, cFilePath);
    QRegularExpression regex("([^ \t]+)(?:[ \t]+(.+))?");
    for(const Divided &divided : divideds.map()) {    //遍历所有部分
        for(const Divided::Part &part : divided.parts) {    //遍历所有行
            int len = part.text.length();
            int start = SearchText(part.text, 0, len, SearchNonspcFn);
            if(start == -1)     //如果为-1说明该行为空
                continue;
            int end = SearchTextReverse(part.text, 0, len, SearchNonspcFn) + 1;

            QString str = part.text.mid(start, end - start);
            QRegularExpressionMatch match = regex.match(str);
            if(!match.hasMatch())
                continue;
            FileAction key{ fileId, match.captured(1) };
            QString val = match.captured(2);
            int index = mResult.mActions.keyIndex(key);
            if(index != -1) {    //检查是否重复
                const DeclarePos &dp = mResult.mActionsInfo[index].declarePos;
                mResult.mIssues << Issue(Issue::Error, tr("Redefintition of \"%1\" (First definition is at row %2, col %3)")
                    .arg(key.str, QString::number(dp.row + 1), QString::number(dp.col + 1)),
                    cFilePath, part.row, part.col);
                continue;
            }
            mResult.mActions.appendKey(key);
            mResult.mActionsInfo << ActionInfo{ DeclarePos{ part.row, part.col }, val };
        }
    }
}
void Parser_::parseProd(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds) {
    int fileId = mResult.mFiles.keyIndex(cFilePath);
    checkDividedArg(tag, divideds, cFilePath);
    for(const Divided &divided : divideds.map()) {
        for(const Divided::Part &part : divided.parts) {
            int len = part.text.length();
            int end = 0;
            struct SplitElement { QString str; int col; };
            QList<SplitElement> list;
            for(int start = SearchText(part.text, 0, len, SearchNonspcFn);
                start != -1;
                start = SearchText(part.text, end + 1, len, SearchNonspcFn))
            {
                end = SearchText(part.text, start + 1, len, SearchSpcFn);
                if(end == -1)
                    end = len;
                
                QString mid = part.text.mid(start, end - start);
                list << SplitElement{ mid, start };
            }
            int size = list.size();
            if(size == 0)
                continue;

            mResult.mHasProd = true;    //标记有产生式
            if(size == 1) {     //检查元素个数
                mResult.mIssues << Issue(Issue::Error, tr("Lack of \"->\" to represent production"), 
                    cFilePath, part.row, part.col);
                continue;
            }
            if(list[1].str != "->") {   //检查第二个位置是否为"->"
                mResult.mIssues << Issue(Issue::Error, tr("Should be \"->\" instead of \"%1\"").arg(list[1].str), 
                    cFilePath, part.row, part.col + list[1].col);
                continue;
            }

            bool hasErr = false;
            Prod prod;
            //得到产生式的左部
            const SplitElement &leftElement = list[0];
            int leftDigit = mResult.mSymbols.keyIndex({ fileId, leftElement.str });
            if(leftDigit == -1) {
                mResult.mIssues << Issue(Issue::Error, tr("Unknown symbol \"%1\"").arg(leftElement.str), 
                    cFilePath, part.row, part.col + leftElement.col);
                hasErr = true;
            } else if(isTerminal(leftDigit)) {
                mResult.mIssues << Issue(Issue::Error, tr("The left of the production cannot be a terminal"), 
                    cFilePath, part.row, part.col);
            } else mResult.mSymbolsInfo[leftDigit].used = true;     //标记该符号使用过
            //得到产生式的右部
            for(auto iter = list.cbegin() + 2; iter != list.cend(); ++iter) {     //从第三个开始遍历
                const SplitElement &se = *iter;
                const QString &str = se.str;
                if(str.length() >= 4 && str.left(2) == "__" && str.right(2) == "__") {  //如果是语义动作
                    QString name = str.mid(2, str.length() - 4); //得到语义动作的名称
                    int action = mResult.mActions.keyIndex({ fileId, name });
                    if(action == -1) {    //检查是否存在语义动作
                        mResult.mIssues << Issue(Issue::Error, tr("Unknown semantic action \"%1\"").arg(name), 
                            cFilePath, part.row, part.col + se.col);
                        hasErr = true;
                        continue;
                    }
                    if(!hasErr) {
                        prod.actions << ProdAction{ prod.symbols.size(), action };
                        mResult.mActionsInfo[action].used = true;   //标记该语义动作使用过
                    }
                } else {    //否则，是词法符号
                    int digit = mResult.mSymbols.keyIndex({ fileId, str });
                    if(digit == -1) {   //检查是否存在该符号
                        mResult.mIssues << Issue(Issue::Error, tr("Unknown symbol \"%1\"").arg(str), 
                            cFilePath, part.row, part.col + se.col);
                        hasErr = true;
                        continue;
                    }
                    if(!hasErr) { 
                        prod.symbols << digit;  //向产生式右部添加符号
                        mResult.mSymbolsInfo[digit].used = true;    //标记该符号使用过
                    }
                }
            }
            //如果没有错误，则将该产生式的右部附加到产生式左部符号的Prods中
            if(!hasErr) {
                Prods &prods = mResult.mProds[leftDigit];
                if(prods.contains(prod)) {
                    mResult.mIssues << Issue(Issue::Warning, tr("Redefinition of the production"), 
                        cFilePath, part.row, part.col);
                } else {
                    prods << prod;
                }
            }
        }
    }
}
