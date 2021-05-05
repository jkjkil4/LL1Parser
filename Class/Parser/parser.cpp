#include "parser.h"

// #include <QDebug>

Parser::Parser(const QString &filePath, QWidget *dialogParent, QObject *parent) 
    : QObject(parent), mDialogParent(dialogParent)
{
    CanonicalFilePath cFilePath(filePath);

    auto tryParse = [this](const QString &tag, ParseFn fn) {    //尝试分析指定标记
        for(auto fileIter = mResult.mFilesDivideds.begin(); fileIter != mResult.mFilesDivideds.end(); ++fileIter) {   //遍历所有文件
            const CanonicalFilePath &cFilePath = fileIter.key();   //文件名
            auto iter = fileIter->find(tag);    //查找标记
            if(iter != fileIter->end()) {
                (this->*fn)(cFilePath, tag, *iter);  //分析标记
                iter->setParsed(true);
            }
        }
    };

    divideAndImportFile(cFilePath);
    int fileId = mResult.mFiles.keyIndex(cFilePath);

    if(mResult.mIssues.hasError())
        goto End;

    mResult.mSymbols.appendKey(FileSymbol{ fileId, "S" });  //开始符号
    mResult.mSymbolsInfo.append(SymbolInfo{ DeclarePos(), true });
    tryParse("Nonterminal", &Parser::parseSymbol);   //分析非终结符
    mResult.mNonterminalMaxIndex = mResult.mSymbolsInfo.size() - 1;

    mResult.mSymbols.appendKey(FileSymbol{ fileId, "$" });  //结束符号
    mResult.mSymbolsInfo.append(SymbolInfo{ DeclarePos(), true });
    tryParse("Terminal", &Parser::parseSymbol);      //分析终结符
    mResult.mTerminalMaxIndex = mResult.mSymbolsInfo.size() - 1;

    tryParse("Action", &Parser::parseAction);
    tryParse("Production", &Parser::parseProd);
    if(!mResult.mHasProd)
        mResult.mIssues << Issue(Issue::Error, tr("Cannot find any production"));
    
    if(mResult.mIssues.hasError())
        goto End;

    parseSymbolsNil();
    if(mResult.mIssues.hasError())
        goto End;
    
    parseFirstSet();
    parseFollowSet();
    parseSelectSets();
    if(mResult.mIssues.hasError())
        goto End;

    tryParse("JS", &Parser::parseJS);
    if(mResult.mIssues.hasError()) 
        goto End;
    tryParse("Output", &Parser::parseOutput);
    if(mResult.mIssues.hasError()) 
        goto End;

    {//检查是否有未使用的符号
        int index = 0;
        for(const SymbolInfo &info : mResult.mSymbolsInfo) {
            if(!info.used) {
                FileSymbol fs = mResult.mSymbols.indexKey(index);
                mResult.mIssues << Issue(Issue::Warning, tr("Unused symbol \"%1\"").arg(fs.str),
                    mResult.mFiles.indexKey(fs.fileId), info.declarePos.row, info.declarePos.col);
            }
            index++;
        }
    }

    {//检查是否有未使用的语义动作
        int index = 0;
        for(const ActionInfo &info : mResult.mActionsInfo) {
            if(!info.used) {
                FileAction fa = mResult.mActions.indexKey(index);
                mResult.mIssues << Issue(Issue::Warning, tr("Unused semantic action \"%1\"").arg(fa.str),
                    mResult.mFiles.indexKey(fa.fileId), info.declarePos.row, info.declarePos.col);
            }
        }
    }

    {//检查是否有未知标记
        QString trUnkTag = tr("Unknown tag \"%1\"");
        for(auto iter = mResult.mFilesDivideds.cbegin(); iter != mResult.mFilesDivideds.cend(); ++iter) {  //遍历所有文件
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
    
    End:;
}
Parser::~Parser() {
    for(JS *js : mResult.mJS)
        delete js;
}

bool Parser::divideFile(const CanonicalFilePath &cFilePath, const QString &basePath) {
    if(mResult.mFiles.contains(cFilePath))  //如果处理过该文件，则return
        return false;
    mResult.mFiles.appendKey(cFilePath);    //标记处理过该文件

    QFile file(cFilePath);   //文件
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) { //尝试打开文件
        mResult.mIssues << Issue(Issue::Error, tr("Cannot open the file \"%1\"").arg(QFileInfo(cFilePath).fileName()), basePath);
        return false;
    }

    QTextStream in(&file);  //用于读取文件
    QRegularExpression regex("%\\[(.*?)(?:\\:(.*?)){0,1}\\]%");     //正则表达式
    MapDivideds &mapDivideds = mResult.mFilesDivideds[cFilePath];    //得到该文件对应的mapDivideds
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
void Parser::divideAndImportFile(const CanonicalFilePath &cFilePath, const QString &basePath) {
    if(!divideFile(cFilePath, basePath))    //尝试分割，若返回值为false则return
        return;

    int fileId = mResult.mFiles.keyIndex(cFilePath);
    MapDivideds &md = mResult.mFilesDivideds[cFilePath];   //得到分割的内容
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
            divideAndImportFile(aImportFilePath, cFilePath);
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
bool Parser::checkDividedArg(const QString &tag, const Divideds &divideds, const QString &filePath) {
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
int Parser::findTrueRowByDividedRow(const Divideds &divideds, int dividedRow) {
    for(const Divided &divided : divideds.map()) {
        if(dividedRow < divided.parts.size())
            return divided.parts[dividedRow].row;
        dividedRow -= divided.parts.size();
    }
    return -1;
}
Parser::FileElement Parser::transFileElement(int fileId, const QString &name) {
    int resFileId = fileId;
    QString resName;
    int splitIndex = name.indexOf(':');
    if(splitIndex == -1) {
        resName = name;
    } else {
        QString resFileAbbre = name.left(splitIndex);
        auto iter = mResult.mFileRels.find({ fileId, resFileAbbre });
        if(iter == mResult.mFileRels.end())
            return FileElement{ -1, "" };

        resFileId = (*iter).fileId;
        resName = name.right(name.length() - splitIndex - 1);
    }
    return FileElement{ resFileId, resName };
}

void Parser::parseSymbol(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds) {
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
void Parser::parseAction(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds) {
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
            mResult.mActionsInfo << ActionInfo{ DeclarePos{ part.row, part.col }, val, false };
        }
    }
}
void Parser::parseProd(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds) {
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
            prod.declarePos = DeclarePos{ part.row, part.col };
            //得到产生式的左部
            const SplitElement &leftElement = list[0];
            int leftDigit = mResult.mSymbols.keyIndex(transFileElement(fileId, leftElement.str));
            if(leftDigit == -1) {
                mResult.mIssues << Issue(Issue::Error, tr("Unknown symbol \"%1\"").arg(leftElement.str), 
                    cFilePath, part.row, part.col + leftElement.col);
                hasErr = true;
            } else if(mResult.isTerminal(leftDigit)) {
                mResult.mIssues << Issue(Issue::Error, tr("The left of the production cannot be a terminal"), 
                    cFilePath, part.row, part.col);
            } else mResult.mSymbolsInfo[leftDigit].used = true;     //标记该符号使用过
            //得到产生式的右部
            for(auto iter = list.cbegin() + 2; iter != list.cend(); ++iter) {     //从第三个开始遍历
                const SplitElement &se = *iter;
                const QString &str = se.str;
                if(str.length() >= 4 && str.left(2) == "__" && str.right(2) == "__") {  //如果是语义动作
                    QString name = str.mid(2, str.length() - 4); //得到语义动作的名称
                    int action = mResult.mActions.keyIndex(transFileElement(fileId, name));
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
                    int digit = mResult.mSymbols.keyIndex(transFileElement(fileId, str));
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
                int indexProd = prods.indexOf(prod);
                if(indexProd != -1) {  //检查是否重复
                    const DeclarePos &dp = prods[indexProd].declarePos;
                    mResult.mIssues << Issue(Issue::Warning, tr("Redefinition of the production (First definition is at row %1, col %2)")
                        .arg(QString::number(dp.row + 1), QString::number(dp.col + 1)), 
                        cFilePath, part.row, part.col);
                } else {
                    prods << prod;
                }
            }
        }
    }
}

void Parser::parseSymbolsNil() {
    enum NilState { Cannot = 0, Unknown = 1, Can = 2 };
    QVector<NilState> tmpVecNil;
    int size = mResult.mNonterminalMaxIndex + 1;
    tmpVecNil.resize(size);
    tmpVecNil.fill(Unknown);

    bool isContinue, isChanged;
    do {
        isContinue = false;
        isChanged = false;

        for(auto iter = mResult.mProds.cbegin(); iter != mResult.mProds.cend(); ++iter) {  //遍历所有产生式
            if(tmpVecNil[iter.key()] == Unknown) {  //如果没有判断完成，则将isContinue设置为true，否则跳过本次循环
                isContinue = true;
            } else continue;

            NilState totalState = Cannot;  //对于所有产生式右部的结果
            for(const Prod &prod : iter.value()) {   //遍历所有产生式右部
                const SymbolVec &prodSymbols = prod.symbols;
                if(prodSymbols.isEmpty()) {    //如果产生式可以直接推导出空串
                    totalState = Can;    //标记为Can
                    isChanged = true;               //标记发生变化
                    break;
                }

                NilState state = Can;   //对于当前产生式右部的结果
                for(int symbol : prodSymbols) {     //遍历该产生式右部的所有符号
                    //如果当前位置的符号 不是非终结符 或者 是非终结符但是不能推导出空串
                    if(!mResult.isNonterminal(symbol) || tmpVecNil[symbol] == Cannot) {
                        state = Cannot;     //标记为Cannot
                        break;
                    }

                    //如果不知道当前位置的符号能否推导出空串
                    if(tmpVecNil[symbol] == Unknown) {
                        state = Unknown;    //标记为Unknown
                        break;
                    }
                }

                if(state > totalState) totalState = state;  //对是否覆盖总结果进行判断
                if(totalState == Can) break;    //对是结束该内部循环进行判断
            }

            if(totalState != Unknown) {     //如果判断出结果，则进行设置
                tmpVecNil[iter.key()] = totalState;
                isChanged = true;
            }
        }

        if(isContinue && !isChanged) {  //当出现死循环时结束分析
            struct Format
            {
                QString color;
                QString lighterColor;
            };
            QStringList formats = { "#dd3333", "#3333dd", "#33dd33" };
            QString formatTerminal = "#000000";

            QString text;
            QTextStream ts(&text);
            ts.setCodec("UTF-8");
            for(auto iter = mResult.mProds.cbegin(); iter != mResult.mProds.cend(); ++iter) {  //遍历所有产生式
                const QString &symbolStr = mResult.mSymbols.indexKey(iter.key()).format();
                for(const Prod &prod : iter.value()) {   //遍历所有的产生式右部
                    const SymbolVec &prodSymbols = prod.symbols;
                    //换行
                    ts << "<br>";

                    //产生式左侧
                    ts << "<font color=\"" << formats[tmpVecNil[iter.key()]] << "\">" << symbolStr << "</font> ->";

                    for(int symbol : prodSymbols) {    //遍历该产生式右部
                        QString &format = (mResult.isNonterminal(symbol) ? formats[tmpVecNil[symbol]] : formatTerminal);
                        ts << " <font color=\"" << format << "\">";
                        ts << mResult.mSymbols.indexKey(symbol).format();
                        ts << "</font>";
                    }
                }
            }
            auto item = NewPlwiFn([text] {
                QTextEdit *edit = new QTextEdit;
                edit->setAttribute(Qt::WA_DeleteOnClose);
                edit->setWindowTitle(tr("Error infomation"));
                edit->setReadOnly(true);
                edit->setHtml(text);
                edit->insertPlainText(tr("green - can be empty string\n"
                                         "red - cannot be empty string\n"
                                         "blue - unknown"));
                edit->resize(600, 400);
                edit->setMinimumSize(300, 200);
                j::SetPointSize(edit, 10);
                j::SetFamily(edit, fontSourceCodePro.mFamily);
                edit->show();
            });
            mResult.mIssues << Issue(Issue::Error, tr("Appear left recursive"), item);
            return;
        }
    } while(isContinue);

    mResult.mSymbolsNil.resize(size);
    repeat(int, i, size)
        mResult.mSymbolsNil[i] = (tmpVecNil[i] == Can);
}
void Parser::parseFirstSet() {
    struct TmpSymbol
    {
        TmpSymbol(int digit) : digit(digit), isQuote(false) {}
        TmpSymbol(int digit, bool isQuote) : digit(digit), isQuote(isQuote) {}
        int digit;
        bool isQuote;

        //在QVector<>::contains中使用
        inline bool operator==(const TmpSymbol &other) const { return digit == other.digit && isQuote == other.isQuote; }
    };
    typedef QVector<TmpSymbol> TmpFirstSet;
    QVector<TmpFirstSet> vecTmpFirstSet;
    int size = mResult.mNonterminalMaxIndex + 1;
    vecTmpFirstSet.resize(size);

    //初始化解析
    for(auto iter = mResult.mProds.cbegin(); iter != mResult.mProds.cend(); ++iter) { //遍历所有产生式
        TmpFirstSet &tmpSet = vecTmpFirstSet[iter.key()];
        for(const Prod &prod : iter.value()) {    //遍历所有的产生式右部
            for(int symbol : prod.symbols) {    //遍历产生式右部的所有符号
                if(mResult.isTerminal(symbol)) {    //如果该符号为终结符，则将其添加到当前FIRST集中并跳出该循环
                    if(!tmpSet.contains(symbol))
                        tmpSet << symbol;
                    break;
                }

                //将该符号以FIRST集的形式添加到当前FIRST集中
                TmpSymbol quoteSymbol(symbol, true);
                if(!tmpSet.contains(quoteSymbol))
                    tmpSet << quoteSymbol;

                if(!mResult.mSymbolsNil[symbol])     //如果该符号无法推导出空串，则跳出该循环
                    break;
            }
        }
    }

    bool isContinue;
    do {//解析
        isContinue = false;

        for(TmpFirstSet &tmpFirstSet : vecTmpFirstSet) {    //遍历所有FIRST集
            for(int i = 0; i < tmpFirstSet.size(); i++) {   //遍历FIRST集中的所有内容
                const TmpSymbol &tmpSymbol = tmpFirstSet[i];
                if(tmpSymbol.isQuote) { //如果是以FIRST集的形式
                    isContinue = true;
                    for(const TmpSymbol &otherSymbol : vecTmpFirstSet[tmpSymbol.digit]) { //遍历并展开FIRST集
                        if(!tmpFirstSet.contains(otherSymbol))
                            tmpFirstSet << otherSymbol;
                    }
                    tmpFirstSet.removeAt(i);
                    i--;
                }
            }
        }
    } while(isContinue);

    mResult.mFirstSet.resize(size);
    for(int i = 0; i < size; i++) {
        SymbolVec &firstSet = mResult.mFirstSet[i];
        const TmpFirstSet &tmpFirstSet = vecTmpFirstSet[i];

        firstSet.reserve(tmpFirstSet.size());
        for(const TmpSymbol &tmpSymbol : tmpFirstSet)
            firstSet << tmpSymbol.digit;
    }
}
void Parser::parseFollowSet() {
    struct TmpSymbol
    {
        enum Type { Symbol, FirstSet, FollowSet } type;
        int digit;
        TmpSymbol(Type state, int digit) : type(state), digit(digit) {}

        //这并不是unused，只是Qt没看出来(在QVector<>::contains中使用)
        inline bool operator==(const TmpSymbol &other) const { return type == other.type && digit == other.digit; }
    };
    typedef QVector<TmpSymbol> TmpFollowSet;
    QVector<TmpFollowSet> vecTmpFollowSet;
    int size = mResult.mNonterminalMaxIndex + 1;
    vecTmpFollowSet.resize(size);

    //初始化解析
    vecTmpFollowSet[0] << TmpSymbol(TmpSymbol::Symbol, mResult.mNonterminalMaxIndex + 1);    //向"S"的FOLLOW集中加入"$"
    for(auto iter = mResult.mProds.cbegin(); iter != mResult.mProds.cend(); ++iter) { //遍历所有产生式
        for(const Prod &prod : iter.value()) {    //遍历所有的产生式右部
            const SymbolVec &prodSymbols = prod.symbols;
            QVector<TmpSymbol> vec;
            vec << TmpSymbol(TmpSymbol::FollowSet, iter.key());     //将产生式左部以FOLLOW集的形式添加到vec中
            for(auto prodIter = prodSymbols.crbegin(); prodIter != prodSymbols.crend(); ++prodIter) {   //反向遍历产生式右部
                //如果该符号是非终结符，则将vec中的所有内容加入到该符号的FOLLOW集中
                if(mResult.isNonterminal(*prodIter)) {
                    TmpFollowSet &tmpFollowSet = vecTmpFollowSet[*prodIter];
                    for(const TmpSymbol &tmpSymbol : vec) {
                        if(!tmpFollowSet.contains(tmpSymbol))
                            tmpFollowSet << tmpSymbol;
                    }
                }

                //如果该符号是终结符或者不能推导出空串，则清除vec的内容
                if(mResult.isTerminal(*prodIter) || !mResult.mSymbolsNil[*prodIter])
                    vec.clear();

                //如果该符号是非终结符，则将其以FIRST集的形式添加到vec中，否则将其直接添加到vec中
                TmpSymbol tmpSymbol(mResult.isNonterminal(*prodIter) ? TmpSymbol::FirstSet : TmpSymbol::Symbol, *prodIter);
                if(!vec.contains(tmpSymbol))
                    vec << tmpSymbol;
            }
        }
    }

    bool isContinue;
    do {//解析
        isContinue = false;

        for(TmpFollowSet &tmpFollowSet : vecTmpFollowSet) {     //遍历所有的FOLLOW集
            for(int i = 0; i < tmpFollowSet.size(); i++) {      //遍历该FOLLOW集的所有内容
                TmpSymbol &tmpSymbol = tmpFollowSet[i];
                if(tmpSymbol.type == TmpSymbol::FirstSet) {     //如果是以FIRST集的形式
                    isContinue = true;
                    for(int symbol : mResult.mFirstSet[tmpSymbol.digit]) {    //遍历并展开FIRST集
                        TmpSymbol otherSymbol(TmpSymbol::Symbol, symbol);
                        if(!tmpFollowSet.contains(otherSymbol))
                            tmpFollowSet << otherSymbol;
                    }
                    tmpFollowSet.removeAt(i);
                    i--;
                } else if(tmpSymbol.type == TmpSymbol::FollowSet) {     //如果是以FOLLOW集的形式
                    isContinue = true;
                    for(const TmpSymbol &otherSymbol : vecTmpFollowSet[tmpSymbol.digit]) {    //遍历并展开FOLLOW集
                        if(!tmpFollowSet.contains(otherSymbol))
                            tmpFollowSet << otherSymbol;
                    }
                    tmpFollowSet.removeAt(i);
                    i--;
                }
            }
        }
    } while(isContinue);

    mResult.mFollowSet.resize(size);
    for(int i = 0; i < size; i++) {
        SymbolVec &followSet = mResult.mFollowSet[i];
        const TmpFollowSet &tmpFollowSet = vecTmpFollowSet[i];

        followSet.reserve(tmpFollowSet.size());
        for(const TmpSymbol &tmpSymbol : tmpFollowSet)
            followSet << tmpSymbol.digit;
    }
}
void Parser::parseSelectSets() {
    int size = mResult.mNonterminalMaxIndex + 1;
    mResult.mSelectSets.resize(size);

    //解析
    for(auto iter = mResult.mProds.cbegin(); iter != mResult.mProds.cend(); ++iter) {     //遍历所有产生式
        for(const Prod &prod : iter.value()) {   //遍历所有的产生式右部
            const SymbolVec &prodSymbols = prod.symbols;
            SymbolVec firstSet;
            bool hasNil = true;

            for(int symbol : prodSymbols) {    //遍历产生式右部的所有符号
                if(mResult.isNonterminal(symbol)) {     //如果该符号是非终结符
                    for(int otherSymbol : mResult.mFirstSet[symbol]) {    //遍历该非终结符的FIRST集
                        if(!firstSet.contains(otherSymbol))
                            firstSet << otherSymbol;
                    }
                } else {
                    if(!firstSet.contains(symbol))
                        firstSet << symbol;
                }

                if(mResult.isTerminal(symbol) || !mResult.mSymbolsNil[symbol]) {  //如果该符号是终结符或者无法推导出空串
                    hasNil = false;
                    break;
                }
            }

            if(hasNil) {
                for(int symbol : mResult.mFollowSet[iter.key()]) {    //遍历产生式左部符号的FOLLOW集
                    if(!firstSet.contains(symbol))
                        firstSet << symbol;
                }
            }
            mResult.mSelectSets[iter.key()] << SelectSet{ firstSet, prod };
        }
    }

    //检查SELECT集是否出现交叉
    QVector<SymbolVec> vecIntersectedSymbols;
    vecIntersectedSymbols.resize(size);
    bool hasIntersection = false;
    for(int i = 0; i < size; i++) {     //遍历所有的非终结符
        SymbolVec appearedSymbols;
        SymbolVec &intersectedSymbols = vecIntersectedSymbols[i];
        for(const SelectSet &selectSet : mResult.mSelectSets[i]) {   //遍历该非终结符的所有SELECT集
            for(int symbol : selectSet.symbols) {   //遍历该SELECT集的所有符号
                if(appearedSymbols.contains(symbol)) {
                    hasIntersection = true;
                    if(!intersectedSymbols.contains(symbol))
                        intersectedSymbols << symbol;
                    break;
                } else appearedSymbols << symbol;
            }
        }
    }
    if(hasIntersection) {   //如果出现交叉
        mResult.mIssues << Issue(Issue::Error, tr("SELECT set has intersections"), 
            new PLWI_ShowHtmlText(tr("Error infomation"), mResult.formatSelectSet(true, &vecIntersectedSymbols)));
    }
}

void Parser::parseJS(const CanonicalFilePath &cFilePath, const QString &tag, const Divideds &divideds) {
    int fileId = mResult.mFiles.keyIndex(cFilePath);
    checkDividedArg(tag, divideds, cFilePath);

    JS *js = mResult.mJS[fileId] = new JS;     //初始化JS
    js->obj.setNonterminalMaxIndex(mResult.mNonterminalMaxIndex);
    js->obj.setTerminalMaxIndex(mResult.mTerminalMaxIndex);
    //传入对象和数据
    QJSValue jsObj = js->engine.newQObject(&js->obj);
    js->engine.globalObject().setProperty("lp", jsObj);
    jsObj.setProperty("nonterminalMaxIndex", mResult.mNonterminalMaxIndex);
    jsObj.setProperty("terminalMaxIndex", mResult.mTerminalMaxIndex);

    int fileCount = mResult.mFiles.map().size();
    int symbolCount = mResult.mSymbols.map().size();
    int nonterminalCount = mResult.mNonterminalMaxIndex + 1;
    int actionCount = mResult.mActions.map().size();

    //传入文件列表
    QJSValue jsFileArray = js->engine.newArray((uint)fileCount);
    repeat(int, i, fileCount)
        jsFileArray.setProperty((uint)i, mResult.mFiles.indexKey(i).text());
    jsObj.setProperty("arrFiles", jsFileArray);

    //传入符号列表
    QJSValue jsSymbolArray = js->engine.newArray((uint)symbolCount);
    repeat(int, i, symbolCount) {
        FileSymbol fs = mResult.mSymbols.indexKey(i);
        QJSValue jsSymbol = js->engine.newObject();
        jsSymbol.setProperty("fileId", fs.fileId);
        jsSymbol.setProperty("str", fs.str);
        jsSymbolArray.setProperty((uint)i, jsSymbol);
    }
    jsObj.setProperty("arrSymbols", jsSymbolArray);

    //传入语义动作
    QJSValue jsActionArray = js->engine.newArray((uint)actionCount);
    repeat(int, i, actionCount) {
        FileAction fa = mResult.mActions.indexKey(i);
        const ActionInfo &info = mResult.mActionsInfo[i];
        QJSValue jsAction = js->engine.newObject();
        jsAction.setProperty("fileId", fa.fileId);
        jsAction.setProperty("name", fa.str);
        jsAction.setProperty("text", info.val);
        jsActionArray.setProperty((uint)i, jsAction);
    }
    jsObj.setProperty("arrActions", jsActionArray);

    //传入产生式
    QJSValue jsProdsArray = js->engine.newArray((uint)symbolCount);
    repeat(int, i, symbolCount) {   //遍历所有的符号
        Prods &prods = mResult.mProds[i];     //该符号的所有产生式
        int prodsSize = prods.size();   //产生式数量
        QJSValue jsProdArray = js->engine.newArray((uint)prodsSize);
        int index = 0;
        for(auto iter = prods.begin(); iter != prods.end(); ++iter) {   //遍历该符号的所有产生式
            const Prod &prod = *iter;   //其中一个产生式
            int size = prod.symbols.size() + prod.actions.size();   //结果大小
            QJSValue jsSymbolArray = js->engine.newArray((uint)size);

            //将产生式符号和语义动作加入到jsSymbolArray中
            int start = 0;
            int pos = 0;
            auto addElement = [js, &jsSymbolArray, &pos](int value, bool isSymbol) {    //lambda，用于添加元素
                QJSValue jsElement = js->engine.newObject();
                jsElement.setProperty("value", value);
                jsElement.setProperty("isSymbol", isSymbol);
                jsSymbolArray.setProperty((uint)pos, jsElement);
                pos++;
            };
            for(const ProdAction &action : prod.actions) {
                for(int i = start; i < action.pos; i++)
                    addElement(prod.symbols[i], true);
                addElement(action.id, false);
                start = action.pos;
            }
            for(int i = start; i < prod.symbols.size(); i++)
                addElement(prod.symbols[i], true);

            jsProdArray.setProperty((uint)index, jsSymbolArray);
            
            index++;
        }
        jsProdsArray.setProperty((uint)i, jsProdArray);
    }
    jsObj.setProperty("arrProds", jsProdsArray);

    //传入能否推导出空串
    QJSValue jsNilArray = js->engine.newArray((uint)nonterminalCount);
    repeat(int, i, nonterminalCount)
        jsNilArray.setProperty((uint)i, mResult.mSymbolsNil[i]);
    jsObj.setProperty("arrSymbolNil", jsNilArray);

    //传入FIRST集
    QJSValue jsFirstSetArray = js->engine.newArray((uint)nonterminalCount);
    for(int i = 0; i < nonterminalCount; i++) {   //遍历所有非终结符
        const SymbolVec &symbols = mResult.mFirstSet[i];        //该符号的FIRST集
        int count = symbols.size();
        QJSValue jsSymbols = js->engine.newArray((uint)count);
        repeat(int, j, count)
            jsSymbols.setProperty((uint)j, symbols[j]);
        jsFirstSetArray.setProperty((uint)i, jsSymbols);
    }
    jsObj.setProperty("arrFirstSet", jsFirstSetArray);

    //传入FOLLOW集
    QJSValue jsFollowSetArray = js->engine.newArray((uint)nonterminalCount);
    for(int i = 0; i < nonterminalCount; i++) {     //遍历所有非终结符
        const SymbolVec &symbols = mResult.mFollowSet[i];   //该符号的FOLLOW集
        int count = symbols.size();
        QJSValue jsSymbols = js->engine.newArray((uint)count);
        repeat(int, j, count)
            jsSymbols.setProperty((uint)j, symbols[j]);
        jsFollowSetArray.setProperty((uint)i, jsSymbols);
    }
    jsObj.setProperty("arrFollowSet", jsFollowSetArray);

    //传入SELECT集
    QJSValue jsSelectSetsArray = js->engine.newArray((uint)nonterminalCount);
    for(int i = 0; i < nonterminalCount; i++) {     //遍历所有非终结符
        const SelectSets &selectSets = mResult.mSelectSets[i];  //该符号的所有SELECT集
        int selectSetCount = selectSets.size();     //该符号的SELECT集数量
        QJSValue jsSelectSets = js->engine.newArray((uint)selectSetCount);
        repeat(int, j, selectSetCount) {    //遍历该符号的所有SELECT集
            const SelectSet &selectSet = selectSets[j];           //当前SELECT集
            int prodSize = selectSet.prod.symbols.size() + selectSet.prod.actions.size();   //产生式元素数量
            int symbolsSize = selectSet.symbols.size();     //SELECT集符号数量

            QJSValue jsSelectSet = js->engine.newObject();
            QJSValue jsProd = js->engine.newArray((uint)prodSize);
            QJSValue jsSymbols = js->engine.newArray((uint)symbolsSize);

            //将产生式的所有元素添加到jsProd中
            int index = 0;
            int start = 0;
            auto fnAddElement = [js, &jsProd, &index](int value, bool isSymbol) {   //lambda，用于添加元素
                QJSValue jsElement = js->engine.newObject();
                jsElement.setProperty("value", value);
                jsElement.setProperty("isSymbol", isSymbol);
                jsProd.setProperty((uint)index, jsElement);
                index++;
            };
            for(const ProdAction &action : selectSet.prod.actions) {
                for(int k = start; k < action.pos; k++)
                    fnAddElement(selectSet.prod.symbols[k], true);
                fnAddElement(action.id, false);
                start = action.pos;
            }
            for(int k = start; k < selectSet.prod.symbols.size(); k++)
                fnAddElement(selectSet.prod.symbols[k], true);

            repeat(int, k, symbolsSize) //遍历当前SELECT集
                jsSymbols.setProperty((uint)k, selectSet.symbols[k]);

            jsSelectSet.setProperty("prod", jsProd);
            jsSelectSet.setProperty("symbols", jsSymbols);
            jsSelectSets.setProperty((uint)j, jsSelectSet);
        }
        jsSelectSetsArray.setProperty((uint)i, jsSelectSets);
    }
    jsObj.setProperty("arrSelectSets", jsSelectSetsArray);

    QString trJSTerminate = tr("JS runs too long in first execution,\n"
                            "Do you want to force it to terminate?\n"
                            "If you continue waiting, it may finish.");
    
    //将分割的字符串合并为整体
    QString total;
    for(const Divided &divided : divideds.map())
        for(const Divided::Part &part : divided.parts)
            total += part.text + '\n';

    //执行js脚本    
    DelayedVerifyDialog dialog(cFilePath + '\n' + trJSTerminate, tr("Terminate"), mDialogParent);
    dialog.delayedVerify(800);
    bool terminate = false;
    connect(&dialog, &DelayedVerifyDialog::accepted, [this, &cFilePath, js, &terminate] {
        js->engine.onTerminateEvaluate();
        terminate = true;
        mResult.mIssues << Issue(Issue::Error, tr("JS terminated in first execution"), cFilePath);
    });
    QJSValue result = js->engine.terminableEvaluate(total);
    if(terminate)
        return;
    if(result.isError()) {
        int dividedRow = result.property("lineNumber").toInt() - 1;
        int row = findTrueRowByDividedRow(divideds, dividedRow);
        mResult.mIssues << Issue(Issue::Error, tr("JS error: \" %1 \"").arg(result.toString()), cFilePath, row, -1);
    }
}
void Parser::parseOutput(const CanonicalFilePath &cFilePath, const QString &, const Divideds &divideds) {
    int fileId = mResult.mFiles.keyIndex(cFilePath);
    QString path = QFileInfo(cFilePath).path();
    QString trJSCallTerminate = tr("JS runs too long when calling \"%1\",\n"
                                "Do you want to force it to terminate?\n"
                                "If you continue waiting, it may finish.");

    QString mpd = QFileInfo(mResult.mFiles.indexKey(0)).path();     //项目目录 Main Project Directory
    QRegularExpression ruleOutputFormat("#\\[(.*?)(?:\\:(.*?)){0,1}\\]#");  //正则表达式，用于匹配格式化操作
    const QMap<QString, Divided> &map = divideds.map();
    for(auto iter = map.cbegin(); iter != map.cend(); ++iter) {    //遍历所有Divided，得到所有要输出的内容
        const Divided &divided = iter.value();
        QString outputPath = iter.key().simplified().trimmed().replace("$$MPD", mpd);
        if(outputPath.isEmpty()) {
            for(int row : divided.rows)
                mResult.mIssues << Issue(Issue::Error, tr("The output path is empty"), cFilePath, row);
        }
        QString text;   //用于得到当前内容
        bool hasPrev = false;
        for(const Divided::Part &part : divided.parts) {
            if(hasPrev) {
                text += '\n';
            } else hasPrev = true;
            QString res;    //用于得到当前行的结果
            int start = 0;
            QRegularExpressionMatchIterator matchIter = ruleOutputFormat.globalMatch(part.text);    //使用正则表达式匹配该行
            while(matchIter.hasNext()) {    //遍历所有匹配
                QRegularExpressionMatch match = matchIter.next();
                res += part.text.mid(start, match.capturedStart() - start);
                start = match.capturedEnd();
                QString name = match.captured(1);
                QString arg = match.captured(2);
                FileElement element = transFileElement(fileId, arg);

                if(name == "js") {  //如果是调用js
                    bool hasFn = false;     //标记在js中是否有该函数
                    JS *js = mResult.mJS.value(element.fileId, nullptr);
                    if(js) {
                        CanonicalFilePath cOtherFilePath = mResult.mFiles.indexKey(element.fileId);
                        const Divideds &jsDivideds = mResult.mFilesDivideds.value(cOtherFilePath).value("JS");
                        QJSValue jsValueFn = js->engine.globalObject().property(element.str);   //得到arg在js中对应的内容
                        if(jsValueFn.isCallable()) {    //如果可以作为函数调用
                            hasFn = true;

                            DelayedVerifyDialog dialog(cFilePath + '\n' + trJSCallTerminate.arg(arg), tr("Terminate"), mDialogParent);
                            dialog.delayedVerify(800);
                            bool terminate = false;
                            connect(&dialog, &DelayedVerifyDialog::accepted, [this, &cFilePath, &part, start, &arg, js, &terminate] {
                                js->engine.onTerminateCall();
                                terminate = true;
                                mResult.mIssues << Issue(Issue::Error, tr("JS terminated when calling \"%1\"").arg(arg), 
                                    cFilePath, part.row, part.col + start);
                            });
                            QJSValue jsRes = js->engine.terminableCall(jsValueFn);
                            if(terminate)
                                return;

                            if(jsRes.isError()) {   //如果有错，则输出错误信息
                                int dividedRow = jsRes.property("lineNumber").toInt() - 1;
                                int row = findTrueRowByDividedRow(jsDivideds, dividedRow);
                                mResult.mIssues << Issue(Issue::Error, tr("JS error: \" %1 \"").arg(jsRes.toString()), 
                                    cOtherFilePath, row);
                            } else res += jsValueFn.call().toString(); //将调用的结果附加到res中
                        }
                    }
                    if(!hasFn) {    //如果没有对应函数，则报错
                        mResult.mIssues << Issue(Issue::Error, tr("Unknown js function \"%1\"").arg(arg),
                            cFilePath, part.row, part.col + start);
                    }
                } else if(name == "symbol") {   //如果是以符号数字替换
                    int symbol = mResult.mSymbols.keyIndex(element); //查找arg对应的数字
                    if(symbol != -1) {  //如果有对应的数字，则附加到res中并标记使用过，否则报错
                        res += QString::number(symbol);
                        mResult.mSymbolsInfo[symbol].used = true;
                    } else {
                        mResult.mIssues << Issue(Issue::Error, tr("Unknown symbol \"%1\"").arg(arg),
                            cFilePath, part.row, part.col + start);
                    }
                } else {
                    mResult.mIssues << Issue(Issue::Error, tr("Unknown \"%1\"").arg(name),
                        cFilePath, part.row, part.col + start);    //如果name未知，则报错
                }
            }
            res += part.text.mid(start, part.text.length() - start);
            text += res;
        }
        mResult.mOutput << Output{ path, outputPath, text };
    }
}


//-------------Parser::Result------------------
QList<Parser::JSDebugMessage> Parser::Result::jsDebugMessage() const {
    QList<JSDebugMessage> jsDebugMsgList;
    for(auto iter = mJS.cbegin(); iter != mJS.cend(); ++iter) {     //遍历所有文件的js
        const JSObject &obj = (*iter)->obj;
        if(obj.hasDebugMessage())
            jsDebugMsgList << JSDebugMessage{ iter.key(), iter.value()->obj.debugMessage() };
    }
    return jsDebugMsgList;
}

QStringList Parser::Result::output() {
    QMap<CanonicalFilePath, QString> mapOutput;
    for(const Output &output : mOutput) {   //遍历所有的输出文件，合并重复
        QDir dir(output.basePath);
        QString &text = mapOutput[dir.absoluteFilePath(output.path)];
        if(!text.isEmpty())
            text += '\n';
        text += output.text;
    }

    QStringList listPaths;
    for(auto iter = mapOutput.cbegin(); iter != mapOutput.cend(); ++iter) {     //遍历所有合并后的输出文件，并输出
        const QString &filePath = iter.key();
        const QString &text = iter.value();
        if(!QDir(QFileInfo(filePath).path()).exists()) {
            mIssues << Issue(Issue::Error, tr("The path of \"%1\" does not exists").arg(filePath));
            continue;
        }
        QFile file(filePath);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            mIssues << Issue(Issue::Error, tr("Cannot write text to file \"%1\"").arg(filePath));
            continue;
        }
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << text;
        file.close();
        listPaths << filePath;
    }

    return listPaths;   //返回成功写入的文件
}

QString Parser::Result::formatFiles() {
    QString result;
    QTextStream ts(&result);
    ts.setCodec("UTF-8");
    
    int count = mFiles.map().size();
    bool hasPrev = false;
    repeat(int, i, count) {     //遍历所有文件
        if(hasPrev) {
            ts << '\n';
        } else hasPrev = true;
        ts << i << " ---> " << mFiles.indexKey(i);
    }

    return result;
}
QString Parser::Result::formatSymbolsNil() {
    QString result;
    QTextStream ts(&result);
    ts.setCodec("UTF-8");

    QVector<int> vecCanBeNil, vecCannotBeNil;
    for(int i = 0; i <= mNonterminalMaxIndex; i++) {    //遍历所有非终结符，检查能否推导出空串
        (mSymbolsNil[i] ? vecCanBeNil : vecCannotBeNil) << i;
    }

    ts << tr("Can be empty string") << ":";
    for(int symbol : vecCanBeNil)   //遍历所有能推导出空串的
        ts << "\n" << mSymbols.indexKey(symbol).format();

    ts << "\n\n" << tr("Cannot be empty string") << ":";
    for(int symbol : vecCannotBeNil)    //遍历所有不能推导出空串的
        ts << "\n" << mSymbols.indexKey(symbol).format();

    return result;
}
QString Parser::Result::formatSet(const QVector<SymbolVec> &vecSet, bool useHtml, bool showNil) {
    QString result;
    QTextStream ts(&result);
    ts.setCodec("UTF-8");

    bool hasPrev = false;
    for(int i = 0; i < vecSet.size(); i++) {    //遍历该集合
        if(hasPrev) {
            ts << (useHtml ? "<br>" : "\n");
        } else hasPrev = true;

        useHtml ? (ts << "<font color=\"blue\">" << mSymbols.indexKey(i).format() << "</font>") : (ts << mSymbols.indexKey(i).format());
        ts << " { ";

        const SymbolVec &set = vecSet[i];
        bool hasPrev2 = false;
        for(int symbol : set) {
            if(hasPrev2) {
                ts << ", ";
            } else hasPrev2 = true;

            useHtml ? (ts << "<font color=\"purple\">" << mSymbols.indexKey(symbol).format() << "</font>") : (ts << mSymbols.indexKey(symbol).format());
        }

        if(showNil && mSymbolsNil[i]) {
            if(hasPrev) ts << ", ";
            ts << (useHtml ? "<font color=\"magenta\">nil</font>" : "nil");
        }

        ts << " }";
    }

    return result;
}
QString Parser::Result::formatSelectSet(bool useHtml, QVector<SymbolVec> *pVecIntersectedSymbols) {
    QString result;
    QTextStream ts(&result);
    ts.setCodec("UTF-8");

    bool hasPrev = false;
    for(int i = 0; i < mSelectSets.size(); i++) {     //遍历所有非终结符
        for(const SelectSet &selectSet : mSelectSets[i]) {  //遍历该非终结符的SELECT集
            if(hasPrev) {
                ts << (useHtml ? "<br>" : "\n");
            } else hasPrev = true;

            useHtml ? (ts << "<font color=\"blue\">" << mSymbols.indexKey(i).format() << "</font>") : (ts << mSymbols.indexKey(i).format());
            ts << " -> ";
            if(selectSet.prod.symbols.isEmpty() && selectSet.prod.actions.isEmpty()) {
                ts << (useHtml ? "<font color=\"magenta\">nil</font> " : "nil ");
            } else {
                int start = 0;
                for(const ProdAction &action : selectSet.prod.actions) {
                    for(int j = start; j < action.pos; j++) {
                        int symbol = selectSet.prod.symbols[j];
                        useHtml ? (ts << "<font color=\"blue\">" << mSymbols.indexKey(symbol).format() << "</font>") : (ts << mSymbols.indexKey(symbol).format());
                        ts << " ";
                    }
                    useHtml ? (ts << "<font color=\"magenta\">" << mActions.indexKey(action.id).format() << "</font>") : (ts << mActions.indexKey(action.id).format());
                    ts << " ";
                    start = action.pos;
                }
                for(int j = start; j < selectSet.prod.symbols.length(); j++) {
                    int symbol = selectSet.prod.symbols[j];
                    useHtml ? (ts << "<font color=\"blue\">" << mSymbols.indexKey(symbol).format() << "</font>") : (ts << mSymbols.indexKey(symbol).format());
                    ts << " ";
                }
            }

            ts << "{ ";
            bool hasPrev2 = false;
            for(int symbol : selectSet.symbols) {   //遍历该SELECT集
                if(hasPrev2) {
                    ts << ", ";
                } else hasPrev2 = true;

                bool isWrong = (pVecIntersectedSymbols && pVecIntersectedSymbols->at(i).contains(symbol));
                if(isWrong) {
                    useHtml ? (ts << "<font color=\"red\"><u>" << mSymbols.indexKey(symbol).format() << "</u></font>")
                            : (ts << mSymbols.indexKey(symbol).format());
                } else {
                    useHtml ? (ts << "<font color=\"purple\">" << mSymbols.indexKey(symbol).format() << "</font>")
                            : (ts << mSymbols.indexKey(symbol).format());
                }
            }
            ts << " }";
        }
    }

    return result;
}
