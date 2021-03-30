#include "texthighlighter.h"

#include <QDebug>

#define ADD_HIGHLIGHT_TAGFN(tag, fn) \
    mMapTags[tag] = mVecFn.size();\
    mVecFn << fn
#define ADD_HIGHLIGHT_FN(fn) \
    mVecFn << fn

TextHighlighter::TextHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    mFormatTagBracket.setForeground(Qt::blue);
    mFormatTagText.setForeground(Qt::magenta);
    mFormatTagArg.setForeground(Qt::darkMagenta);

    mFormatOFBracket.setForeground(Qt::blue);
    mFormatOFText.setForeground(QColor(5, 215, 5));
    mFormatOFArg.setForeground(Qt::darkGreen);

    mFormatProdArrow.setForeground(Qt::darkYellow);
    mFormatProdWrongArrow.setForeground(Qt::red);

    mFormatJSStringWithBracket.setForeground(Qt::darkMagenta);  //JS函数
    mListJSHighlightRules << HighlightRule{ mRuleJSStringWithBracket, mFormatJSStringWithBracket, 1 };

    mFormatJSQObj.setForeground(Qt::magenta);   //JS lp
    mListJSHighlightRules << HighlightRule{ mRuleJSQObj, mFormatJSQObj, 0 };

    mFormatJSNumber.setForeground(Qt::blue);    //JS数字
    mListJSHighlightRules << HighlightRule{ mRuleJSNumber, mFormatJSNumber, 0 };
    mListJSHighlightRules << HighlightRule{ mRuleJSHexNumber, mFormatJSNumber, 0 };

    mFormatJSCommit.setForeground(Qt::darkGreen);   //JS注释
    mFormatJSString.setForeground(QColor(160, 85, 60));     //JS字符串
    mFormatJSStrQuote.setForeground(QColor(255, 128, 128)); //JS字符转义
    mFormatJSRegex.setForeground(QColor(220, 75, 50));

    //JS关键字高亮
    mFormatJSKeyword.setForeground(Qt::darkYellow);
    const QString keywordPatterns[] = {
        QStringLiteral("\\babstract\\b"), QStringLiteral("\\bboolean\\b"), QStringLiteral("\\bbreak\\b"), QStringLiteral("\\bbyte\\b"),
        QStringLiteral("\\bcase\\b"), QStringLiteral("\\bcatch\\b"), QStringLiteral("\\bchar\\b"), QStringLiteral("\\bconst\\b"),
        QStringLiteral("\\bcontinue\\b"), QStringLiteral("\\bdebugger\\b"), QStringLiteral("\\bdefault\\b"), QStringLiteral("\\bdelete\\b"),
        QStringLiteral("\\bdelete\\b"), QStringLiteral("\\bdo\\b"), QStringLiteral("\\bdouble\\b"), QStringLiteral("\\belse\\b"),
        QStringLiteral("\\beval\\b"), QStringLiteral("\\bfalse\\b"), QStringLiteral("\\btrue\\b"), QStringLiteral("\\bfinal\\b"), QStringLiteral("\\bfinally\\b"),
        QStringLiteral("\\bfloat\\b"), QStringLiteral("\\bfor\\b"), QStringLiteral("\\bfunction\\b"), QStringLiteral("\\bgoto\\b"),
        QStringLiteral("\\bif\\b"), QStringLiteral("\\bimplements\\b"), QStringLiteral("\\bin\\b"), QStringLiteral("\\binstanceof\\b"),
        QStringLiteral("\\bint\\b"),QStringLiteral("\\binterface\\b"), QStringLiteral("\\blet\\b"), QStringLiteral("\\blong\\b"),
        QStringLiteral("\\bnative\\b"), QStringLiteral("\\bnew\\b"), QStringLiteral("\\bnull\\b"), QStringLiteral("\\bpackage\\b"),
        QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"), QStringLiteral("\\breturn\\b"),
        QStringLiteral("\\bshort\\b"), QStringLiteral("\\bstatic\\b"), QStringLiteral("\\bswitch\\b"), QStringLiteral("\\bsynchronized\\b"),
        QStringLiteral("\\btry\\b"), QStringLiteral("\\btypeof\\b"), QStringLiteral("\\bvar\\b"), QStringLiteral("\\bvoid\\b"),
        QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\bwhile\\b"), QStringLiteral("\\bwith\\b"), QStringLiteral("\\byield\\b")
    };
    for(const QString &pattern : keywordPatterns)
        mListJSHighlightRules << HighlightRule{ QRegularExpression(pattern), mFormatJSKeyword, 0 };

    //添加处理函数
    ADD_HIGHLIGHT_TAGFN("Production", &TextHighlighter::highlightProduction);
    ADD_HIGHLIGHT_TAGFN("JS", &TextHighlighter::highlightJS);
    ADD_HIGHLIGHT_TAGFN("Output", &TextHighlighter::highlightOutput);
    ADD_HIGHLIGHT_FN(&TextHighlighter::highlightJSCommit);
    ADD_HIGHLIGHT_FN(&TextHighlighter::highlightJSMultiLineCommit);
    ADD_HIGHLIGHT_FN(&TextHighlighter::highlightJSMqString);
    ADD_HIGHLIGHT_FN(&TextHighlighter::highlightJSSqString);
    ADD_HIGHLIGHT_FN(&TextHighlighter::highlightJSRegex);
}
#undef ADD_HIGHLIGHT_TAGFN
#undef ADD_HIGHLIGHT_FN

void TextHighlighter::highlightBlock(const QString &text) {
    HighlightConfig hc(text, indexFn(previousBlockState()));
    int highlightStart = 0;

    //匹配标记
    QRegularExpressionMatchIterator matchIter = mRuleTag.globalMatch(text);
    while(matchIter.hasNext()) {
        QRegularExpressionMatch match = matchIter.next();

        //高亮标记
        setFormat(match.capturedStart(), 2, mFormatTagBracket);     //左侧"%["
        setFormat(match.capturedStart(1), match.capturedLength(1), mFormatTagText); //标记
        setFormat(match.capturedEnd(1), match.capturedEnd() - 2 - match.capturedLength(2) - match.capturedEnd(1), mFormatTagBracket);   //标记和参数间的":"
        setFormat(match.capturedStart(2), match.capturedLength(2), mFormatTagArg);  //参数
        setFormat(match.capturedEnd() - 2, 2, mFormatTagBracket);   //右侧"]%"

        //分割进行高亮
        do {
            hc.prepare(highlightStart, match.capturedStart() - highlightStart);
            FnHighlight prevFn = hc.fn;
            highlight(hc);
            hc.prevFn = prevFn;
            highlightStart = (hc.offset == -1 ? match.capturedEnd() : highlightStart + hc.offset);
        } while(hc.offset != -1);
        hc.prevFn = hc.fn;
        hc.fn = indexFn(tagIndex(match.captured(1)));
    }

    //高亮尾部
    do {
        hc.prepare(highlightStart, text.length() - highlightStart);
        FnHighlight prevFn = hc.fn;
        highlight(hc);
        hc.prevFn = prevFn;
        if(hc.offset != -1)
            highlightStart += hc.offset;
    } while(hc.offset != -1);

    //对特定情况判断是否让fn变回highlightJS
    if(hc.fn == &TextHighlighter::highlightJSCommit)
        hc.fn = &TextHighlighter::highlightJS;
    else if(hc.fn == &TextHighlighter::highlightJSMqString || hc.fn == &TextHighlighter::highlightJSSqString) {
        if(text.right(1) != '\\') 
            hc.fn = &TextHighlighter::highlightJS;
    }
    setCurrentBlockState(mVecFn.indexOf(hc.fn));
}

void TextHighlighter::highlightProduction(HighlightConfig &hc) {
    int nonspcCount = 0;
    bool isInSpc = true;
    for(int i = 0; i < hc.len; i++) {
        QChar ch = hc.text[hc.start + i];
        if(ch != '\t' && ch != ' ') {
            if(!isInSpc)
                continue;
            isInSpc = false;
            nonspcCount++;
            if(nonspcCount == 2) {
                int midLen = 1;
                for(int j = i + 1; j < hc.len; j++) {
                    QChar ch2 = hc.text[hc.start + j];
                    if(ch2 != '\t' && ch2 != ' ') {
                        midLen++;
                    } else break;
                }
                const QTextCharFormat &format = (hc.text.mid(hc.start + i, midLen) == "->" ? mFormatProdArrow : mFormatProdWrongArrow);
                setFormat(hc.start + i, midLen, format);
                break;
            }
        } else isInSpc = true;
    }
}

void TextHighlighter::highlightJS(HighlightConfig &hc) {
    int end = hc.start + hc.len;

    //匹配子高亮
    QRegularExpressionMatch match = mRuleJSInnerHighlight.match(hc.text, hc.start);
    if(match.hasMatch() && match.capturedEnd() <= end) {
        QString captured = match.captured();
        if(captured.length() == 2 && captured[0] == '/' && captured[1] != '*' && captured[1] != '/') {  //如果满足正则表达式的开头
            //高亮正则表达式
            hc.fn = &TextHighlighter::highlightJSRegex;
            hc.offset = match.capturedStart() - hc.start;
            end = hc.start + hc.offset;
        } else {
            //根据mMapJSFn查找的结果进行高亮
            auto iter = mMapJSFn.find(captured);
            if(iter != mMapJSFn.end()) {
                hc.fn = *iter;
                hc.offset = match.capturedStart() - hc.start;
                end = hc.start + hc.offset;
            }
        }
    }

    //JS高亮
    for(const HighlightRule &rule : mListJSHighlightRules) {
        QRegularExpressionMatchIterator matchIter = rule.pattern.globalMatch(hc.text, hc.start);
        while(matchIter.hasNext()) {
            QRegularExpressionMatch match = matchIter.next();
            if(match.capturedEnd() > end)
                break;
            setFormat(match.capturedStart(rule.nth), match.capturedLength(rule.nth), rule.format);
        }
    }
}

void TextHighlighter::highlightOutput(HighlightConfig &hc) {
    int end = hc.start + hc.len;
    //匹配
    QRegularExpressionMatchIterator matchIter = mRuleOutputFormat.globalMatch(hc.text);
    while(matchIter.hasNext()) {
        QRegularExpressionMatch match = matchIter.next();
        if(match.capturedEnd() > end)
            break;
        //高亮
        setFormat(match.capturedStart(), 2, mFormatOFBracket);     //左侧"#["
        setFormat(match.capturedStart(1), match.capturedLength(1), mFormatOFText); //名称
        setFormat(match.capturedEnd(1), match.capturedEnd() - 2 - match.capturedLength(2) - match.capturedEnd(1), mFormatOFBracket);   //标记和参数间的":"
        setFormat(match.capturedStart(2), match.capturedLength(2), mFormatOFArg);  //参数
        setFormat(match.capturedEnd() - 2, 2, mFormatOFBracket);   //右侧"]#"
    }
}

void TextHighlighter::highlightJSCommit(HighlightConfig &hc) {
    setFormat(hc.start, hc.len, mFormatJSCommit);
}
void TextHighlighter::highlightJSMultiLineCommit(HighlightConfig &hc) {
    int len = hc.len;
    QRegularExpressionMatch match = mRuleJSMultiLineCommitEnd.match(hc.text, hc.fn == hc.prevFn ? hc.start : hc.start + 2);
    if(match.hasMatch() && match.capturedEnd() <= hc.start + hc.len) {
        len = hc.offset = match.capturedEnd() - hc.start;
        hc.fn = &TextHighlighter::highlightJS;
    }
    setFormat(hc.start, len, mFormatJSCommit);
}

void TextHighlighter::highlightJSString(HighlightConfig &hc, const QRegularExpression &regexQuoteOrEnd, QChar chEnd) {
    int start = hc.start;
    int end = hc.start + hc.len;
    QRegularExpressionMatch match = regexQuoteOrEnd.match(hc.text, hc.fn == hc.prevFn ? hc.start : hc.start + 1);
    while(match.hasMatch() && match.capturedEnd() <= end) {
        setFormat(start, match.capturedStart() - start, mFormatJSString);
        if(match.captured() == chEnd) {  //判断是否为字符串结束
            setFormat(match.capturedStart(), match.capturedLength(), mFormatJSString);
            hc.offset = match.capturedEnd() - hc.start;
            hc.fn = &TextHighlighter::highlightJS;
            return;
        }
        setFormat(match.capturedStart(), 2, mFormatJSStrQuote);
        start = match.capturedEnd() + 1;    //"+1"是为了忽略转义后的符号
        match = mRuleJSMqStringQuoteOrEnd.match(hc.text, start);
    }
    setFormat(start, end - start, mFormatJSString);
}

void TextHighlighter::highlightJSRegex(HighlightConfig &hc) {
    hc.offset = 1;
    hc.fn = &TextHighlighter::highlightJS;

    //从hc.start的前一个字符向前遍历，找到第一个不为'\t'或' '的字符，若该字符不能在正则表达式前存在，则结束
    bool checkLeft = true;
    for(int i = hc.start - 1; i >= 0; i++) {
        QChar ch = hc.text[i];  //该位置的字符
        if(ch != '\t' && ch != ' ') {
            if(IsLetter(ch) || IsDigit(ch) || ch == ']')
                checkLeft = false;
            break;
        }
    }
    if(!checkLeft) return;

    //从hc.start的后一个字符向后遍历，进行相应判断
    int end = hc.start + hc.len;
    bool hasEnd = false;
    bool optionEnded = false;
    QVector<int> vecQuote;
    int lastPos = -1;
    for(int i = hc.start + 1; i < end; i++) {
        QChar ch = hc.text[i];
        if(hasEnd) {
            if(optionEnded) {
                if(ch != '\t' && ch != ' ') {   //如果该字符串不为'\t'或' '
                    if(IsLetter(ch) || IsDigit(ch))     //如果该字符不能在正则表达式后存在，则结束
                        return;
                    break;
                }
            } else {
                if(ch == '\t' || ch == ' ') {   //如果该字符为'\t'或' '
                    optionEnded = true; //标记结束了对正则表达式选项的分析
                } else if(ch == 'g' || ch == 'i' || ch == 'm') { //如果该字符为正则表达式的选项
                    lastPos = i;
                } else if(IsLetter(ch) || IsDigit(ch)) {  //如果该字符为其他字母或数字，则结束
                    return;
                } else break;
            }
        } else {
            switch(ch.toLatin1()) {
            case '\\':  //转义
                vecQuote << i;
                i++;    //自增以跳过转义符号后的一个字符
                break;
            case '/':   //结尾符号或注释
                if(i != end - 1 && hc.text[i + 1] == '/') return; //如果找不到结尾符号或者找到"//"，则结束
                hasEnd = true;
                lastPos = i;
                break;
            }
        }
    }
    if(!hasEnd) return;   //如果没有结尾，则结束

    //遍历转义并高亮
    int start = hc.start;
    int regexEnd = lastPos + 1;
    for(int index : vecQuote) {
        setFormat(start, index - start, mFormatJSRegex);
        setFormat(index, 2, mFormatJSStrQuote);
        start = index + 2;
    }
    setFormat(start, regexEnd - start, mFormatJSRegex);
    hc.offset = regexEnd - hc.start;
}

int TextHighlighter::tagIndex(const QString &tag) {
    auto iter = mMapTags.find(tag);
    return (iter == mMapTags.end() ? -1 : *iter);
}
TextHighlighter::FnHighlight TextHighlighter::indexFn(int index) {
    if(index < 0 || index >= mVecFn.size())
        return nullptr;
    return mVecFn[index];
}

void TextHighlighter::highlight(HighlightConfig &hc) {
    if(!hc.fn) return;
    (this->*hc.fn)(hc);
}
