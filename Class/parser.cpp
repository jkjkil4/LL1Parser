#include "parser.h"

QList<Parser::Issue> Parser::issues;
bool Parser::hasProd = false;
QMap<QString, Parser::Divided> Parser::mapDivided;
QMap<Parser::Symbol, int> Parser::mapSymbols;
int Parser::symbolsMaxIndex = 0;
int Parser::nonterminalMaxIndex = -1;
int Parser::terminalMaxIndex = -1;
Parser::ProdsMap Parser::mapProds;
QVector<bool> Parser::vecNil;
QVector<Parser::SymbolVec> Parser::vecFirstSet, Parser::vecFollowSet;
QVector<Parser::SelectSets> Parser::vecSelectSets;
Parser::JS* Parser::js = nullptr;
QString Parser::jsDebugMessage;

void Parser::divide(QTextDocument *doc) {
    QRegularExpression regExp("%\\[(.*?)\\]%");     //正则表达式
    int count = doc->lineCount();

    Divided *pDivided = nullptr;     //默认Divided
    repeat(int, i, count) {     //遍历文档的所有行
        QString line = doc->findBlockByLineNumber(i).text();    //当前行
        int start = 0;
        QRegularExpressionMatch match = regExp.match(line);     //使用正则表达式获取内容
        while(match.hasMatch()) {   //重复直到正则表达式获取不到内容
            QString mid = line.mid(start, match.capturedStart() - start);
            if(pDivided) pDivided->parts << Divided::Part{ i, mid };

            start = match.capturedEnd();
            pDivided = &mapDivided[match.captured(1)];
            pDivided->rows << i;
            match = regExp.match(line, match.capturedEnd());
        }
        QString right = line.right(line.length() - start);
        if(pDivided) pDivided->parts.append(Divided::Part{ i, right });
    }
}

#define TRY_PARSE(key, fn) {            \
    auto iter = mapDivided.find(key);   \
    if(iter != mapDivided.end()) {      \
        fn(*iter);                      \
        iter->parsed = true;            \
    }                                   \
}
void Parser::parse(QTextDocument *doc) {
    clear();
    divide(doc);

    appendSymbol(Symbol::Nonterminal, "S");
    TRY_PARSE("Nonterminal", parseNonterminal);
    nonterminalMaxIndex = symbolsMaxIndex - 1;
    appendSymbol(Symbol::Terminal, "$");
    TRY_PARSE("Terminal", parseTerminal);
    terminalMaxIndex = symbolsMaxIndex - 1;

    TRY_PARSE("Production", parseProduction);
    if(!hasProd)
        issues << Issue(Issue::Error, tr("Cannot find any production"));

    if(hasError()) goto End;

    parseNil();
    if(hasError()) goto End;

    parseFirstSet();
    parseFollowSet();
    parseSelectSet();

    TRY_PARSE("JS", parseJs);

    {//检查是否有未知标记
        QString trStr = tr("Unknown tag \"%1\"");
        for(auto iter = mapDivided.begin(); iter != mapDivided.end(); ++iter) {
            if(!iter->parsed) {
                for(int row : iter->rows) {
                    issues << Issue(Issue::Warning, trStr.arg(iter.key()), row);
                }
            }
        }
    }

    End:
    j::SafeDelete(js);
}
#undef TRY_PARSE

void Parser::parseTerminal(const Divided &divided) {
    for(const Divided::Part &part : divided.parts) {    //遍历所有行
        QStringList list = part.text.split(' ', QString::SkipEmptyParts);   //分割字符串
        int phrase = -1;
        for(QString &str : list) {  //遍历分割的结果
            phrase++;

            if(str == 'S' || str == '$') {  //如果与自带符号冲突
                issues << Issue(Issue::Error, tr("Cannot use \"%1\" as symbol").arg(str), part.row, phrase);
                continue;
            }

            auto iter = mapSymbols.find(str);
            if(iter != mapSymbols.end()) {  //如果该符号已经存在
                if(iter.key().type == Symbol::Nonterminal) {
                    issues << Issue(Issue::Error, tr("Symbol \"%1\" is already an nonterminal").arg(str), part.row, phrase);
                } else {
                    issues << Issue(Issue::Warning, tr("Redefinition of \"%1\"").arg(str),part.row, phrase);
                }
                continue;
            }
            appendSymbol(Symbol::Terminal, str);
        }
    }
}
void Parser::parseNonterminal(const Divided &divided) {
    for(const Divided::Part &part : divided.parts) {    //遍历所有行
        QStringList list = part.text.split(' ', QString::SkipEmptyParts);   //分割字符串
        int phrase = -1;
        for(QString &str : list) {  //遍历分割的结果
            phrase++;

            if(str == 'S' || str == '$') {  //如果与自带符号冲突
                issues << Issue(Issue::Error, tr("Cannot use \"%1\" as symbol").arg(str), part.row, phrase);
                continue;
            }

            auto iter = mapSymbols.find(str);
            if(iter != mapSymbols.end()) {  //如果该符号已经存在
                if(iter.key().type == Symbol::Terminal) {
                    issues << Issue(Issue::Error, tr("Symbol \"%1\" is already a terminal").arg(str), part.row, phrase);
                } else {
                    issues << Issue(Issue::Warning, tr("Redefinition of \"%1\"").arg(str), part.row, phrase);
                }
                continue;
            }
            appendSymbol(Symbol::Nonterminal, str);
        }
    }
}
void Parser::parseProduction(const Divided &divided) {
    for(const Divided::Part &part : divided.parts) {    //遍历所有行
        QStringList list = part.text.split(' ', QString::SkipEmptyParts);   //分割字符串
        int size = list.size();
        if(size == 0)
            continue;

        hasProd = true;

        if(size == 1) {
            issues << Issue(Issue::Error, tr("Lack of \"->\" to represent production"), part.row);
            continue;
        }
        if(list[1] != "->") {
            issues << Issue(Issue::Error, tr("Should be \"->\" instead of \"%1\"").arg(list[1]), part.row, 1);
            continue;
        }

        bool hasErr = false;
        SymbolVec prod;
        //得到产生式的左部
        int leftDigit = mapSymbols.value(list[0], -1);
        if(leftDigit == -1) {
            issues << Issue(Issue::Error, tr("Unknown symbol \"%1\"").arg(list[0]), part.row, 0);
            hasErr = true;
        } else if(isTerminal(leftDigit)) {
            issues << Issue(Issue::Error, tr("The left of the production cannot be a terminal"), part.row, 0);
        }
        //得到产生式的右部
        int phrase = 1;
        for(auto iter = list.begin() + 2; iter != list.end(); ++iter) {     //从第三个开始遍历
            phrase++;
            int digit = mapSymbols.value(*iter, -1);
            if(digit == -1) {
                issues << Issue(Issue::Error, tr("Unknown symbol \"%1\"").arg(*iter), part.row, phrase);
                hasErr = true;
                continue;
            }
            if(!hasErr) prod << digit;
        }
        //如果没有错误，则将该产生式的右部附加到产生式左部符号的Prods中
        if(!hasErr) {
            Prods &prods = mapProds[leftDigit];
            if(prods.contains(prod)) {
                issues << Issue(Issue::Warning, tr("Redefinition of the production"), part.row);
            } else {
                prods << prod;
            }
        }
    }
}
void Parser::parseJs(const Divided &divided) {
    //初始化QJSEngine
    j::SafeDelete(js);
    js = new JS;

    //设置数据
    js->object.setNonterminalMaxIndex(nonterminalMaxIndex);
    js->object.setTerminalMaxIndex(terminalMaxIndex);

    //传入对象
    QJSValue jsObjVal = js->engine.newQObject(&js->object);
    js->engine.globalObject().setProperty("lp", jsObjVal);

    //传入数据
    int symbolCount = mapSymbols.size();
    jsObjVal.setProperty("symbolCount", symbolCount);
    jsObjVal.setProperty("nonterminalMaxIndex", nonterminalMaxIndex);
    jsObjVal.setProperty("terminalMaxIndex", terminalMaxIndex);

    //传入符号列表
    QJSValue jsSymbolArray = js->engine.newArray(symbolCount);
    repeat(int, i, symbolCount)
        jsSymbolArray.setProperty(i, mapSymbols.key(i).str);
    jsObjVal.setProperty("arrSymbols", jsSymbolArray);

    //传入产生式
    QJSValue jsProdsArray = js->engine.newArray(symbolCount);
    repeat(int, i, symbolCount) {   //遍历所有的符号
        Prods &prods = mapProds[i];     //该符号的所有产生式
        int prodsSize = prods.size();   //产生式数量
        QJSValue jsProdArray = js->engine.newArray(prodsSize);
        int index = 0;
        for(auto iter = prods.begin(); iter != prods.end(); ++iter) {   //遍历该符号的所有产生式
            SymbolVec &symbols = *iter;         //其中一个产生式
            int symbolsSize = symbols.size();   //产生式右部大小
            QJSValue jsSymbolArray = js->engine.newArray(symbolsSize);
            for(int j = 0; j < symbolsSize; j++)    //遍历该产生式右部
                jsSymbolArray.setProperty(j, symbols[j]);
            jsProdArray.setProperty(index, jsSymbolArray);
            
            index++;
        }
        jsProdsArray.setProperty(i, jsProdArray);
    }
    jsObjVal.setProperty("arrProds", jsProdsArray);

    //传入能否推导出空串
    QJSValue jsNilArray = js->engine.newArray(symbolCount);
    repeat(int, i, symbolCount)
        jsNilArray.setProperty(i, vecNil[i]);
    jsObjVal.setProperty("arrNil", jsNilArray);

    //传入FIRST集
    QJSValue jsFirstSetArray = js->engine.newArray(nonterminalMaxIndex + 1);
    for(int i = 0; i <= nonterminalMaxIndex; i++) {   //遍历所有非终结符
        SymbolVec &symbols = vecFirstSet[i];        //该符号的FIRST集
        int count = symbols.size();
        QJSValue jsSymbols = js->engine.newArray(count);
        repeat(int, j, count)
            jsSymbols.setProperty(j, symbols[j]);
        jsFirstSetArray.setProperty(i, jsSymbols);
    }
    jsObjVal.setProperty("arrFirstSet", jsFirstSetArray);

    //传入FOLLOW集
    QJSValue jsFollowSetArray = js->engine.newArray(nonterminalMaxIndex + 1);
    for(int i = 0; i <= nonterminalMaxIndex; i++) {     //遍历所有非终结符
        SymbolVec &symbols = vecFollowSet[i];   //该符号的FOLLOW集
        int count = symbols.size();
        QJSValue jsSymbols = js->engine.newArray(count);
        repeat(int, j, count)
            jsSymbols.setProperty(j, symbols[j]);
        jsFollowSetArray.setProperty(i, jsSymbols);
    }
    jsObjVal.setProperty("arrFollowSet", jsFollowSetArray);

    //传入SELECT集
    QJSValue jsSelectSetsArray = js->engine.newArray(nonterminalMaxIndex + 1);
    for(int i = 0; i <= nonterminalMaxIndex; i++) {     //遍历所有非终结符
        SelectSets &selectSets = vecSelectSets[i];  //该符号的所有SELECT集
        int selectSetCount = selectSets.size();     //该符号的SELECT集数量
        QJSValue jsSelectSets = js->engine.newArray(selectSetCount);
        repeat(int, j, selectSetCount) {    //遍历该符号的所有SELECT集
            SelectSet &selectSet = selectSets[j];           //当前SELECT集
            int prodSize = selectSet.prod.size();           //产生式符号数量
            int symbolsSize = selectSet.symbols.size();     //SELECT集符号数量

            QJSValue jsSelectSet = js->engine.newObject();
            QJSValue jsProd = js->engine.newArray(prodSize);
            QJSValue jsSymbols = js->engine.newArray(symbolsSize);
            repeat(int, k, prodSize)    //遍历当前产生式
                jsProd.setProperty(k, selectSet.prod[k]);
            repeat(int, k, symbolsSize) //遍历当前SELECT集
                jsSymbols.setProperty(k, selectSet.symbols[k]);
                
            jsSelectSet.setProperty("prod", jsProd);
            jsSelectSet.setProperty("symbols", jsSymbols);
            jsSelectSets.setProperty(j, jsSelectSet);
        }
        jsSelectSetsArray.setProperty(i, jsSelectSets);
    }
    jsObjVal.setProperty("arrSelectSets", jsSelectSetsArray);

    //将分割的字符串合并为整体
    QString all;
    QTextStream ts(&all);
    ts.setCodec("UTF-8");
    for(const Divided::Part &part : divided.parts)
        ts << part.text << '\n';

    //执行js脚本
    QJSValue result = js->engine.evaluate(all);
    if(result.isError()) {
        int index = result.property("lineNumber").toInt() - 1;
        int row = (index >= 0 && index < divided.parts.size()) ? divided.parts[index].row : -1;
        issues << Issue(Issue::Error, tr("JS error: \" %1 \"").arg(result.toString()), row, -1);
    }

    //调试信息
    if(js->object.hasDebugMessage())
        jsDebugMessage = js->object.debugMessage();
}

void Parser::parseNil() {
    enum NilState { Cannot = 0, Unknown = 1, Can = 2 };
    QVector<NilState> tmpVecNil;
    int size = nonterminalMaxIndex + 1;
    tmpVecNil.resize(size);
    tmpVecNil.fill(Unknown);

    bool isContinue, isChanged;
    do {
        isContinue = false;
        isChanged = false;

        for(auto iter = mapProds.begin(); iter != mapProds.end(); ++iter) {  //遍历所有产生式
            if(tmpVecNil[iter.key()] == Unknown) {  //如果没有判断完成，则将isContinue设置为true，否则跳过本次循环
                isContinue = true;
            } else continue;

            NilState totalState = Cannot;  //对于所有产生式右部的结果
            for(SymbolVec &prod : iter.value()) {   //遍历所有产生式右部
                if(prod.isEmpty()) {    //如果产生式可以直接推导出空串
                    totalState = Can;    //标记为Can
                    isChanged = true;               //标记发生变化
                    break;
                }

                NilState state = Can;   //对于当前产生式右部的结果
                for(int symbol : prod) {     //遍历该产生式右部的所有符号
                    //如果当前位置的符号 不是非终结符 或者 是非终结符但是不能推导出空串
                    if(!isNonterminal(symbol) || tmpVecNil[symbol] == Cannot) {
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
            bool hasPrev = false;
            for(auto iter = mapProds.begin(); iter != mapProds.end(); ++iter) {  //遍历所有产生式
                const QString &symbolStr = mapSymbols.key(iter.key()).str;
                for(SymbolVec &prod : iter.value()) {   //遍历所有的产生式右部
                    //换行
                    if(hasPrev) {
                        ts << "<br>";
                    } else hasPrev = true;

                    //产生式左侧
                    ts << "<font color=\"" << formats[tmpVecNil[iter.key()]] << "\">" << symbolStr << "</font> ->";

                    for(int symbol : prod) {    //遍历该产生式右部
                        QString &format = (isNonterminal(symbol) ? formats[tmpVecNil[symbol]] : formatTerminal);
                        ts << " <font color=\"" << format << "\">";
                        ts << mapSymbols.key(symbol).str;
                        ts << "</font>";
                    }
                }
            }
            issues << Issue(Issue::Error, tr("Appear left recursive") + tr("(Double click to show detail)"), 
                -1, -1, { (int)UserRole::ShowHtmlText, tr("Error infomation"), text });
            return;
        }
    } while(isContinue);

    vecNil.resize(size);
    repeat(int, i, size) {
        vecNil[i] = (tmpVecNil[i] == Can);
    }
}
void Parser::parseFirstSet() {
    struct TmpSymbol
    {
        TmpSymbol(int digit) : digit(digit), isQuote(false) {}
        TmpSymbol(int digit, bool isQuote) : digit(digit), isQuote(isQuote) {}
        int digit;
        bool isQuote;

        //这并不是unused，只是Qt没看出来(在QVector<>::contains中使用)
        inline bool operator==(const TmpSymbol &other) const { return digit == other.digit && isQuote == other.isQuote; }
    };
    typedef QVector<TmpSymbol> TmpFirstSet;
    QVector<TmpFirstSet> vecTmpFirstSet;
    int size = nonterminalMaxIndex + 1;
    vecTmpFirstSet.resize(size);

    //初始化解析
    for(auto iter = mapProds.begin(); iter != mapProds.end(); ++iter) { //遍历所有产生式
        TmpFirstSet &tmpSet = vecTmpFirstSet[iter.key()];
        for(SymbolVec &prod : iter.value()) {    //遍历所有的产生式右部
            for(int symbol : prod) {    //遍历产生式右部的所有符号
                if(isTerminal(symbol)) {    //如果该符号为终结符，则将其添加到当前FIRST集中并跳出该循环
                    if(!tmpSet.contains(symbol))
                        tmpSet << symbol;
                    break;
                }

                //将该符号以FIRST集的形式添加到当前FIRST集中
                TmpSymbol quoteSymbol(symbol, true);
                if(!tmpSet.contains(quoteSymbol))
                    tmpSet << quoteSymbol;

                if(!vecNil[symbol])     //如果该符号无法推导出空串，则跳出该循环
                    break;
            }
        }
    }

    bool isContinue;
    do {//解析
        isContinue = false;

        for(TmpFirstSet &tmpFirstSet : vecTmpFirstSet) {    //遍历所有FIRST集
            for(int i = 0; i < tmpFirstSet.size(); i++) {   //遍历FIRST集中的所有内容
                TmpSymbol &tmpSymbol = tmpFirstSet[i];
                if(tmpSymbol.isQuote) { //如果是以FIRST集的形式
                    isContinue = true;
                    for(TmpSymbol &otherSymbol : vecTmpFirstSet[tmpSymbol.digit]) { //遍历并展开FIRST集
                        if(!tmpFirstSet.contains(otherSymbol))
                            tmpFirstSet << otherSymbol;
                    }
                    tmpFirstSet.removeAt(i);
                    i--;
                }
            }
        }
    } while(isContinue);

    vecFirstSet.resize(size);
    for(int i = 0; i < size; i++) {
        SymbolVec &firstSet = vecFirstSet[i];
        TmpFirstSet &tmpFirstSet = vecTmpFirstSet[i];

        firstSet.reserve(tmpFirstSet.size());
        for(TmpSymbol &tmpSymbol : tmpFirstSet)
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
    int size = nonterminalMaxIndex + 1;
    vecTmpFollowSet.resize(size);

    //初始化解析
    vecTmpFollowSet[0] << TmpSymbol(TmpSymbol::Symbol, nonterminalMaxIndex + 1);    //向"S"的FOLLOW集中加入"$"
    for(auto iter = mapProds.begin(); iter != mapProds.end(); ++iter) { //遍历所有产生式
        for(SymbolVec &prod : iter.value()) {    //遍历所有的产生式右部
            QVector<TmpSymbol> vec;
            vec << TmpSymbol(TmpSymbol::FollowSet, iter.key());     //将产生式左部以FOLLOW集的形式添加到vec中
            for(auto prodIter = prod.rbegin(); prodIter != prod.rend(); ++prodIter) {   //反向遍历产生式右部
                //如果该符号是非终结符，则将vec中的所有内容加入到该符号的FOLLOW集中
                if(isNonterminal(*prodIter)) {
                    TmpFollowSet &tmpFollowSet = vecTmpFollowSet[*prodIter];
                    for(TmpSymbol &tmpSymbol : vec) {
                        if(!tmpFollowSet.contains(tmpSymbol))
                            tmpFollowSet << tmpSymbol;
                    }
                }

                //如果该符号是终结符或者不能推导出空串，则清除vec的内容
                if(isTerminal(*prodIter) || !vecNil[*prodIter])
                    vec.clear();

                //如果该符号是非终结符，则将其以FIRST集的形式添加到vec中，否则将其直接添加到vec中
                TmpSymbol tmpSymbol(isNonterminal(*prodIter) ? TmpSymbol::FirstSet : TmpSymbol::Symbol, *prodIter);
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
                    for(int symbol : vecFirstSet[tmpSymbol.digit]) {    //遍历并展开FIRST集
                        TmpSymbol otherSymbol(TmpSymbol::Symbol, symbol);
                        if(!tmpFollowSet.contains(otherSymbol))
                            tmpFollowSet << otherSymbol;
                    }
                    tmpFollowSet.removeAt(i);
                    i--;
                } else if(tmpSymbol.type == TmpSymbol::FollowSet) {     //如果是以FOLLOW集的形式
                    isContinue = true;
                    for(TmpSymbol &otherSymbol : vecTmpFollowSet[tmpSymbol.digit]) {    //遍历并展开FOLLOW集
                        if(!tmpFollowSet.contains(otherSymbol))
                            tmpFollowSet << otherSymbol;
                    }
                    tmpFollowSet.removeAt(i);
                    i--;
                }
            }
        }
    } while(isContinue);

    vecFollowSet.resize(size);
    for(int i = 0; i < size; i++) {
        SymbolVec &followSet = vecFollowSet[i];
        TmpFollowSet &tmpFollowSet = vecTmpFollowSet[i];

        followSet.reserve(tmpFollowSet.size());
        for(TmpSymbol &tmpSymbol : tmpFollowSet)
            followSet << tmpSymbol.digit;
    }
}
void Parser::parseSelectSet() {
    int size = nonterminalMaxIndex + 1;
    vecSelectSets.resize(size);

    //解析
    for(auto iter = mapProds.begin(); iter != mapProds.end(); ++iter) {     //遍历所有产生式
        for(SymbolVec &prod : iter.value()) {   //遍历所有的产生式右部
            SymbolVec firstSet;
            bool hasNil = true;

            for(int symbol : prod) {    //遍历产生式右部的所有符号
                if(isNonterminal(symbol)) {     //如果该符号是非终结符
                    for(int otherSymbol : vecFirstSet[symbol]) {    //遍历该非终结符的FIRST集
                        if(!firstSet.contains(otherSymbol))
                            firstSet << otherSymbol;
                    }
                } else {
                    if(!firstSet.contains(symbol))
                        firstSet << symbol;
                }

                if(isTerminal(symbol) || !vecNil[symbol]) {  //如果该符号是终结符或者无法推导出空串
                    hasNil = false;
                    break;
                }
            }

            if(hasNil) {
                for(int symbol : vecFollowSet[iter.key()]) {    //遍历产生式左部符号的FOLLOW集
                    if(!firstSet.contains(symbol))
                        firstSet << symbol;
                }
            }
            vecSelectSets[iter.key()] << SelectSet{ firstSet, prod };
        }
    }

    //检查SELECT集是否出现交叉
    QVector<SymbolVec> vecIntersectedSymbols;
    vecIntersectedSymbols.resize(size);
    bool hasIntersection = false;
    for(int i = 0; i < size; i++) {     //遍历所有的非终结符
        SymbolVec appearedSymbols;
        SymbolVec &intersectedSymbols = vecIntersectedSymbols[i];
        for(SelectSet &selectSet : vecSelectSets[i]) {   //遍历该非终结符的所有SELECT集
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
        QString text = formatSelectSet(true, &vecIntersectedSymbols);
        issues << Issue(Issue::Error, tr("SELECT set has intersections"), -1, -1, 
            { (int)UserRole::ShowHtmlText, tr("Error infomation"), text });
    }
}

void Parser::clear() {
    issues.clear();

    hasProd = false;
    mapDivided.clear();
    mapSymbols.clear();
    symbolsMaxIndex = 0;
    nonterminalMaxIndex = -1;
    terminalMaxIndex = -1;
    mapProds.clear();

    vecNil.clear();
    vecFirstSet.clear();
    vecFollowSet.clear();
    vecSelectSets.clear();

    jsDebugMessage.clear();
}
bool Parser::hasError() {
    for(Issue &issue : issues)
        if(issue.type == Issue::Error)
            return true;
    return false;
}
void Parser::appendSymbol(Symbol::Type type, const QString &str) {
    mapSymbols.insert(Symbol(type, str), symbolsMaxIndex);
    symbolsMaxIndex++;
}

QString Parser::formatProdsMap() {
    QString result;
    QTextStream ts(&result);
    ts.setCodec("UTF-8");

    bool hasPrev = false;
    for(auto iter = mapProds.begin(); iter != mapProds.end(); ++iter) {     //遍历所有的产生式
        QString keyStr = mapSymbols.key(iter.key()).str;
        for(SymbolVec &prod : *iter) {   //遍历所有的产生式右部
            if(hasPrev) {
                ts << "\n";
            } else hasPrev = true;

            ts << keyStr << " ->";

            for(int digit : prod) {
                ts << " " << mapSymbols.key(digit).str;
            }
        }
    }
    return result;
}
QString Parser::formatNilVec() {
    QString result;
    QTextStream ts(&result);
    ts.setCodec("UTF-8");

    QVector<int> vecCanBeNil, vecCannotBeNil;
    for(int i = 0; i <= nonterminalMaxIndex; i++) {
        (vecNil[i] ? vecCanBeNil : vecCannotBeNil) << i;
    }

    ts << tr("Can be empty string") << ":";
    for(int symbol : vecCanBeNil)
        ts << "\n" << mapSymbols.key(symbol).str;

    ts << "\n\n" << tr("Cannot be empty string") << ":";
    for(int symbol : vecCannotBeNil)
        ts << "\n" << mapSymbols.key(symbol).str;

    return result;
}
QString Parser::formatSet(const QVector<SymbolVec> &vecSet, bool useHtml, bool showNil) {
    QString result;
    QTextStream ts(&result);
    ts.setCodec("UTF-8");

    bool hasPrev = false;
    for(int i = 0; i < vecSet.size(); i++) {
        if(hasPrev) {
            ts << (useHtml ? "<br>" : "\n");
        } else hasPrev = true;

        useHtml ? (ts << "<font color=\"blue\">" << mapSymbols.key(i).str << "</font>") : (ts << mapSymbols.key(i).str);
        ts << " { ";

        const SymbolVec &set = vecSet[i];
        bool hasPrev2 = false;
        for(int symbol : set) {
            if(hasPrev2) {
                ts << ", ";
            } else hasPrev2 = true;

            useHtml ? (ts << "<font color=\"purple\">" << mapSymbols.key(symbol).str << "</font>") : (ts << mapSymbols.key(symbol).str);
        }

        if(showNil && vecNil[i]) {
            if(hasPrev) ts << ", ";
            ts << (useHtml ? "<font color=\"magenta\">nil</font>" : "nil");
        }

        ts << " }";
    }

    return result;
}
QString Parser::formatSelectSet(bool useHtml, QVector<SymbolVec> *pVecIntersectedSymbols) {
    QString result;
    QTextStream ts(&result);
    ts.setCodec("UTF-8");

    bool hasPrev = false;
    for(int i = 0; i < vecSelectSets.size(); i++) {     //遍历所有非终结符
        for(SelectSet &selectSet : vecSelectSets[i]) {  //遍历该非终结符的SELECT集
            if(hasPrev) {
                ts << (useHtml ? "<br>" : "\n");
            } else hasPrev = true;

            useHtml ? (ts << "<font color=\"blue\">" << mapSymbols.key(i).str << "</font>") : (ts << mapSymbols.key(i).str);
            ts << " -> ";
            if(selectSet.prod.isEmpty()) {
                ts << (useHtml ? "<font color=\"magenta\">nil</font> " : "nil ");
            } else {
                for(int symbol : selectSet.prod) {  //遍历该SELECT集对应的产生式右部
                    useHtml ? (ts << "<font color=\"blue\">" << mapSymbols.key(symbol).str << "</font>") : (ts << mapSymbols.key(symbol).str);
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
                    useHtml ? (ts << "<font color=\"red\"><u>" << mapSymbols.key(symbol).str << "</u></font>")
                            : (ts << mapSymbols.key(symbol).str);
                } else {
                    useHtml ? (ts << "<font color=\"purple\">" << mapSymbols.key(symbol).str << "</font>")
                            : (ts << mapSymbols.key(symbol).str);
                }
            }
            ts << " }";
        }
    }

    return result;
}
