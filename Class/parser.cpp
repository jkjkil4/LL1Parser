#include "parser.h"

QList<Parser::Issue> Parser::issues;
bool Parser::hasProd = false;
QMap<QString, Parser::Divideds> Parser::mapDivideds;
QMap<Parser::Symbol, int> Parser::mapSymbols;
int Parser::symbolsMaxIndex = 0;
int Parser::nonterminalMaxIndex = -1;
int Parser::terminalMaxIndex = -1;
QMap<QString, int> Parser::mapActions;
QStringList Parser::listActions;
Parser::ProdsMap Parser::mapProds;
QVector<bool> Parser::vecNil;
QVector<Parser::SymbolVec> Parser::vecFirstSet, Parser::vecFollowSet;
QVector<Parser::SelectSets> Parser::vecSelectSets;
Parser::JS* Parser::js = nullptr;
QString Parser::jsDebugMessage;
QList<Parser::Output> Parser::listOutput;

void Parser::divide(QTextDocument *doc) {
    QRegularExpression regExp("%\\[(.*?)(?:\\:(.*?)){0,1}\\]%");     //正则表达式
    int count = doc->lineCount();

    Divided *pDivided = nullptr;     //默认Divided
    repeat(int, i, count) {     //遍历文档的所有行
        QString line = doc->findBlockByLineNumber(i).text();    //当前行
        int start = 0;
        QRegularExpressionMatch match = regExp.match(line);     //使用正则表达式获取内容
        while(match.hasMatch()) {   //重复直到正则表达式获取不到内容
            QString mid = line.mid(start, match.capturedStart() - start);
            if(pDivided && !mid.isEmpty())
                pDivided->parts << Divided::Part{ i, mid };

            start = match.capturedEnd();
            pDivided = &mapDivideds[match.captured(1)][match.captured(2)];
            pDivided->rows << i;
            match = regExp.match(line, match.capturedEnd());
        }
        QString right = line.right(line.length() - start);
        if(pDivided && (!right.isEmpty() || start == 0))
            pDivided->parts.append(Divided::Part{ i, right });
    }
}
bool Parser::checkDividedArg(const QString &tag, const Divideds &divideds) {
    bool hasArg = false;
    for(const Divided &divided : divideds.listDivided) {
        if(divided.arg != "") {
            hasArg = true;
            for(int row : divided.rows)
                issues << Issue(Issue::Warning, tr("The tag \"%1\" does not need to provide any parameter").arg(tag), row);
        }
    }
    return hasArg;
}
int Parser::findTrueRowByDividedRow(const Divideds &divideds, int dividedRow) {
    for(const Divided &divided : divideds.listDivided) {
        if(dividedRow < divided.parts.size())
            return divided.parts[dividedRow].row;
        dividedRow -= divided.parts.size();
    }
    return -1;
}

#define TRY_PARSE(_key, fn) {           \
    auto iter = mapDivideds.find(_key); \
    if(iter != mapDivideds.end()) {     \
        fn(iter.key(), *iter);          \
        iter->parsed = true;            \
    }                                   \
}
void Parser::parse(QTextDocument *doc) {
    clear();
    divide(doc);

    appendSymbol(Symbol::Nonterminal, "S");     //开始符号
    TRY_PARSE("Nonterminal", parseNonterminal);
    nonterminalMaxIndex = symbolsMaxIndex - 1;
    appendSymbol(Symbol::Terminal, "$");        //结束符号
    TRY_PARSE("Terminal", parseTerminal);
    terminalMaxIndex = symbolsMaxIndex - 1;

    TRY_PARSE("Action", parseAction);
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
    if(hasError()) goto End;
    TRY_PARSE("Output", parseOutput);

    {//检查是否有未知标记
        QString trStr = tr("Unknown tag \"%1\"");
        for(auto iter = mapDivideds.begin(); iter != mapDivideds.end(); ++iter) {
            if(!iter->parsed) {
                for(const Divided &divided : iter->listDivided) {
                    for(int row : divided.rows) {
                        issues << Issue(Issue::Warning, trStr.arg(iter.key()), row);
                    }
                }
            }
        }
    }

    End:
    j::SafeDelete(js);
}
#undef TRY_PARSE

void Parser::parseTerminal(const QString &tag, const Divideds &divideds) {
    checkDividedArg(tag, divideds);
    for(const Divided &divided : divideds.listDivided) {
        for(const Divided::Part &part : divided.parts) {    //遍历所有行
            QStringList list = part.text.split(' ', QString::SkipEmptyParts);   //分割字符串
            int phrase = -1;
            for(const QString &str : list) {  //遍历分割的结果
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
}
void Parser::parseNonterminal(const QString &tag, const Divideds &divideds) {
    checkDividedArg(tag, divideds);
    for(const Divided &divided : divideds.listDivided) {
        for(const Divided::Part &part : divided.parts) {    //遍历所有行
            QStringList list = part.text.split(' ', QString::SkipEmptyParts);   //分割字符串
            int phrase = -1;
            for(const QString &str : list) {  //遍历分割的结果
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
}
void Parser::parseAction(const QString &tag, const Divideds &divideds) {
    checkDividedArg(tag, divideds);
    QRegularExpression regex("([^ \t]+)(?:[ \t]+(.+))?");
    for(const Divided &divided : divideds.listDivided) {    //遍历所有部分
        for(const Divided::Part &part : divided.parts) {    //遍历所有行
            int start = -1;
            int len = part.text.length();
            for(int i = 0; i < len; i++) {
                QChar ch = part.text[i];
                if(ch != '\t' && ch != ' ') {
                    start = i;
                    break;
                }
            }
            if(start == -1) continue;
            int end = -1;
            for(int i = len - 1; i >= 0; i--) {
                QChar ch = part.text[i];
                if(ch != '\t' && ch != ' ') {
                    end = i + 1;
                    break;
                }
            }
            QString str = part.text.mid(start, end - start);
            QRegularExpressionMatch match = regex.match(str);
            if(!match.hasMatch()) continue;
            QString key = match.captured(1);
            QString val = match.captured(2);
            if(mapActions.contains(key)) {
                issues << Issue(Issue::Error, tr("Redefinition of \"%1\"").arg(key), part.row);
            } else { 
                mapActions[key] = listActions.size();
                listActions << val;
            }
        }
    }
}
void Parser::parseProduction(const QString &tag, const Divideds &divideds) {
    checkDividedArg(tag, divideds);
    for(const Divided &divided : divideds.listDivided) {
        for(const Divided::Part &part : divided.parts) {    //遍历所有行
            QString text = part.text;
            QStringList list = text.replace('\t', ' ').split(' ', QString::SkipEmptyParts);   //分割字符串
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
            Prod prod;
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
                const QString &str = *iter;
                if(str.length() >= 4 && str.left(2) == "__" && str.right(2) == "__") {  //如果是语义动作
                    QString name = str.mid(2, str.length() - 4); //得到语义动作的名称
                    if(!mapActions.contains(name)) {
                        issues << Issue(Issue::Error, tr("Unknown semantic action \"%1\"").arg(name), part.row, phrase);
                        hasErr = true;
                        continue;
                    }
                    if(!hasErr) prod.actions << ProdAction{ prod.symbols.size(), mapActions[name] };
                } else {    //否则，是词法符号
                    int digit = mapSymbols.value(str, -1);
                    if(digit == -1) {
                        issues << Issue(Issue::Error, tr("Unknown symbol \"%1\"").arg(str), part.row, phrase);
                        hasErr = true;
                        continue;
                    }
                    if(!hasErr) prod.symbols << digit;
                }
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
}
void Parser::parseJs(const QString &tag, const Divideds &divideds) {
    checkDividedArg(tag, divideds);

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
    
    //传入语义动作
    QJSValue jsActionArray = js->engine.newArray(mapActions.size());
    for(auto iter = mapActions.begin(); iter != mapActions.end(); ++iter) {
        QJSValue jsAction = js->engine.newObject();
        jsAction.setProperty("name", iter.key());
        jsAction.setProperty("text", listActions[iter.value()]);
        jsActionArray.setProperty(iter.value(), jsAction);
    }
    jsObjVal.setProperty("arrActions", jsActionArray);

    //传入产生式
    QJSValue jsProdsArray = js->engine.newArray(symbolCount);
    repeat(int, i, symbolCount) {   //遍历所有的符号
        Prods &prods = mapProds[i];     //该符号的所有产生式
        int prodsSize = prods.size();   //产生式数量
        QJSValue jsProdArray = js->engine.newArray(prodsSize);
        int index = 0;
        for(auto iter = prods.begin(); iter != prods.end(); ++iter) {   //遍历该符号的所有产生式
            const Prod &prod = *iter;   //其中一个产生式
            int size = prod.symbols.size() + prod.actions.size();   //结果大小
            QJSValue jsSymbolArray = js->engine.newArray(size);

            //将产生式符号和语义动作加入到jsSymbolArray中
            int start = 0;
            int pos = 0;
            auto addElement = [&jsSymbolArray, &pos](int value, bool isSymbol) {    //lambda，用于添加元素
                QJSValue jsElement = Parser::js->engine.newObject();
                jsElement.setProperty("value", value);
                jsElement.setProperty("isSymbol", isSymbol);
                jsSymbolArray.setProperty(pos, jsElement);
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
        const SymbolVec &symbols = vecFirstSet[i];        //该符号的FIRST集
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
        const SymbolVec &symbols = vecFollowSet[i];   //该符号的FOLLOW集
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
        const SelectSets &selectSets = vecSelectSets[i];  //该符号的所有SELECT集
        int selectSetCount = selectSets.size();     //该符号的SELECT集数量
        QJSValue jsSelectSets = js->engine.newArray(selectSetCount);
        repeat(int, j, selectSetCount) {    //遍历该符号的所有SELECT集
            const SelectSet &selectSet = selectSets[j];           //当前SELECT集
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
    for(const Divided &divided : divideds.listDivided)
        for(const Divided::Part &part : divided.parts)
            ts << part.text << '\n';

    //执行js脚本
    QJSValue result = js->engine.evaluate(all);
    if(result.isError()) {
        int dividedRow = result.property("lineNumber").toInt() - 1;
        int row = findTrueRowByDividedRow(divideds, dividedRow);
        issues << Issue(Issue::Error, tr("JS error: \" %1 \"").arg(result.toString()), row, -1);
    }

    //调试信息
    if(js->object.hasDebugMessage())
        jsDebugMessage = js->object.debugMessage();
}
void Parser::parseOutput(const QString &, const Divideds &divideds) {
    QRegularExpression ruleOutputFormat("#\\[(.*?)(?:\\:(.*?)){0,1}\\]#");  //正则表达式，用于匹配格式化操作
    for(const Divided &divided : divideds.listDivided) {    //遍历所有Divided，得到所有要输出的内容
        QString text;   //用于得到当前内容
        bool hasPrev = false;
        for(const Divided::Part &part : divided.parts) {
            if(hasPrev) {
                text += '\n';
            } else hasPrev = true;
            QString res;    //用于得到当前行的结果
            int start = 0;
            QRegularExpressionMatchIterator matchIter = ruleOutputFormat.globalMatch(part.text);    //使用正则表达式匹配改行
            while(matchIter.hasNext()) {    //遍历所有匹配
                QRegularExpressionMatch match = matchIter.next();
                res += part.text.mid(start, match.capturedStart() - start);
                start = match.capturedEnd();
                QString name = match.captured(1);
                QString arg = match.captured(2);
                if(name == "js") {  //如果是调用js
                    bool hasFn = false;     //标记在js中是否有该函数
                    if(js) {
                        QJSValue jsValueFn = js->engine.globalObject().property(arg);   //得到arg在js中对应的内容
                        if(jsValueFn.isCallable()) {    //如果可以作为函数调用
                            hasFn = true;
                            res += jsValueFn.call().toString(); //将调用的结果附加到res中
                        }
                    }
                    if(!hasFn) {    //如果没有对应函数，则报错
                        issues << Issue(Issue::Error, tr("Unknown js function \"%1\"").arg(arg));
                    }
                } else if(name == "symbol") {   //如果是以符号数字替换
                    int symbol = mapSymbols.value(arg, -1); //查找arg对应的数字
                    if(symbol != -1) {  //如果有对应的数字，则附加到res中，否则报错
                        res += QString::number(symbol);
                    } else issues << Issue(Issue::Error, tr("Unknown symbol \"%1\"").arg(arg));
                } else {
                    issues << Issue(Issue::Error, tr("Unknown \"%1\"").arg(name), part.row);    //如果name未知，则报错
                }
            }
            res += part.text.mid(start, part.text.length() - start);
            text += res;
        }
        listOutput << Output{ divided.arg, text };
    }
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
                for(const Prod &prod : iter.value()) {   //遍历所有的产生式右部
                    const SymbolVec &prodSymbols = prod.symbols;
                    //换行
                    if(hasPrev) {
                        ts << "<br>";
                    } else hasPrev = true;

                    //产生式左侧
                    ts << "<font color=\"" << formats[tmpVecNil[iter.key()]] << "\">" << symbolStr << "</font> ->";

                    for(int symbol : prodSymbols) {    //遍历该产生式右部
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
        for(const Prod &prod : iter.value()) {    //遍历所有的产生式右部
            for(int symbol : prod.symbols) {    //遍历产生式右部的所有符号
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

    vecFirstSet.resize(size);
    for(int i = 0; i < size; i++) {
        SymbolVec &firstSet = vecFirstSet[i];
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
    int size = nonterminalMaxIndex + 1;
    vecTmpFollowSet.resize(size);

    //初始化解析
    vecTmpFollowSet[0] << TmpSymbol(TmpSymbol::Symbol, nonterminalMaxIndex + 1);    //向"S"的FOLLOW集中加入"$"
    for(auto iter = mapProds.begin(); iter != mapProds.end(); ++iter) { //遍历所有产生式
        for(const Prod &prod : iter.value()) {    //遍历所有的产生式右部
            const SymbolVec &prodSymbols = prod.symbols;
            QVector<TmpSymbol> vec;
            vec << TmpSymbol(TmpSymbol::FollowSet, iter.key());     //将产生式左部以FOLLOW集的形式添加到vec中
            for(auto prodIter = prodSymbols.rbegin(); prodIter != prodSymbols.rend(); ++prodIter) {   //反向遍历产生式右部
                //如果该符号是非终结符，则将vec中的所有内容加入到该符号的FOLLOW集中
                if(isNonterminal(*prodIter)) {
                    TmpFollowSet &tmpFollowSet = vecTmpFollowSet[*prodIter];
                    for(const TmpSymbol &tmpSymbol : vec) {
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

    vecFollowSet.resize(size);
    for(int i = 0; i < size; i++) {
        SymbolVec &followSet = vecFollowSet[i];
        const TmpFollowSet &tmpFollowSet = vecTmpFollowSet[i];

        followSet.reserve(tmpFollowSet.size());
        for(const TmpSymbol &tmpSymbol : tmpFollowSet)
            followSet << tmpSymbol.digit;
    }
}
void Parser::parseSelectSet() {
    int size = nonterminalMaxIndex + 1;
    vecSelectSets.resize(size);

    //解析
    for(auto iter = mapProds.begin(); iter != mapProds.end(); ++iter) {     //遍历所有产生式
        for(const Prod &prod : iter.value()) {   //遍历所有的产生式右部
            const SymbolVec &prodSymbols = prod.symbols;
            SymbolVec firstSet;
            bool hasNil = true;

            for(int symbol : prodSymbols) {    //遍历产生式右部的所有符号
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
            vecSelectSets[iter.key()] << SelectSet{ firstSet, prodSymbols };
        }
    }

    //检查SELECT集是否出现交叉
    QVector<SymbolVec> vecIntersectedSymbols;
    vecIntersectedSymbols.resize(size);
    bool hasIntersection = false;
    for(int i = 0; i < size; i++) {     //遍历所有的非终结符
        SymbolVec appearedSymbols;
        SymbolVec &intersectedSymbols = vecIntersectedSymbols[i];
        for(const SelectSet &selectSet : vecSelectSets[i]) {   //遍历该非终结符的所有SELECT集
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
    mapDivideds.clear();
    mapSymbols.clear();
    symbolsMaxIndex = 0;
    nonterminalMaxIndex = -1;
    terminalMaxIndex = -1;
    mapActions.clear();
    listActions.clear();
    mapProds.clear();

    vecNil.clear();
    vecFirstSet.clear();
    vecFollowSet.clear();
    vecSelectSets.clear();

    jsDebugMessage.clear();

    listOutput.clear();
}
bool Parser::hasError() {
    for(const Issue &issue : issues)
        if(issue.type == Issue::Error)
            return true;
    return false;
}
void Parser::appendSymbol(Symbol::Type type, const QString &str) {
    mapSymbols.insert(Symbol(type, str), symbolsMaxIndex);
    symbolsMaxIndex++;
}

QString Parser::outputDir(const QString &projPath) {
    return QFileInfo(projPath).path();
}

void Parser::outputFile(const QString &projPath/*, const QString &projName*/) {
    //输出文件
    if(hasOutputFile()) {
        QString path = outputDir(projPath);
        
        for(const Output &output : listOutput) {    //遍历所有内容并输出
            QFileInfo info(output.filePath);
            QDir dir(path);
            if(!dir.cd(info.path())) {
                issues << Issue(Issue::Error, tr("Cannot write text to file \"%1\"").arg(output.filePath));
                continue;
            }
            QFile file(dir.canonicalPath() + "/" + info.fileName());
            if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                issues << Issue(Issue::Error, tr("Cannot write text to file \"%1\"").arg(output.filePath));
                continue;
            }
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << output.text;
            file.close();
        }
    }
}

// QString Parser::formatProdsMap() {
//     QString result;
//     QTextStream ts(&result);
//     ts.setCodec("UTF-8");
//
//     bool hasPrev = false;
//     for(auto iter = mapProds.begin(); iter != mapProds.end(); ++iter) {     //遍历所有的产生式
//         QString keyStr = mapSymbols.key(iter.key()).str;
//         for(const Prod &prod : *iter) {   //遍历所有的产生式右部
//             if(hasPrev) {
//                 ts << "\n";
//             } else hasPrev = true;
//
//             ts << keyStr << " ->";
//
//             for(int digit : prod.symbols) {
//                 ts << " " << mapSymbols.key(digit).str;
//             }
//         }
//     }
//     return result;
// }
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
        for(const SelectSet &selectSet : vecSelectSets[i]) {  //遍历该非终结符的SELECT集
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
