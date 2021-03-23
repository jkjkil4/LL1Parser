#include "texthighlighter.h"

#include <QDebug>

TextHighlighter::TextHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightRule rule;

    mFormatTagBracket.setForeground(Qt::blue);
    mFormatTagText.setForeground(Qt::magenta);
    mFormatTagArg.setForeground(Qt::darkMagenta);
    mFormatTagArg.setFontWeight(QFont::Bold);

    mFormatOFBracket.setForeground(Qt::blue);
    mFormatOFText.setForeground(Qt::green);
    mFormatOFArg.setForeground(Qt::darkGreen);
    mFormatOFArg.setFontWeight(QFont::Bold);

    mFormatProdArrow.setForeground(Qt::darkYellow);
    mFormatProdWrongArrow.setForeground(Qt::red);

    //JS函数
    mFormatJSStringWithBracket.setForeground(Qt::darkMagenta);
    rule.pattern = mRuleJSStringWithBracket;
    rule.format = mFormatJSStringWithBracket;
    rule.nth = 1;
    mListJSHighlightRules << rule;

    //JS lp
    mFormatJSQObj.setForeground(QColor(255, 128, 128));
    rule.pattern = mRuleJSQObj;
    rule.format = mFormatJSQObj;
    rule.nth = 0;
    mListJSHighlightRules << rule;

    //JS关键字高亮
    mFormatJSKeyword.setForeground(Qt::darkYellow);
    const QString keywordPatterns[] = {
        QStringLiteral("\\babstract\\b"), QStringLiteral("\\bboolean\\b"), QStringLiteral("\\bbreak\\b"), QStringLiteral("\\bbyte\\b"),
        QStringLiteral("\\bcase\\b"), QStringLiteral("\\bcatch\\b"), QStringLiteral("\\bchar\\b"), QStringLiteral("\\bconst\\b"),
        QStringLiteral("\\bcontinue\\b"), QStringLiteral("\\bdebugger\\b"), QStringLiteral("\\bdefault\\b"), QStringLiteral("\\bdelete\\b"),
        QStringLiteral("\\bdelete\\b"), QStringLiteral("\\bdo\\b"), QStringLiteral("\\bdouble\\b"), QStringLiteral("\\belse\\b"),
        QStringLiteral("\\beval\\b"), QStringLiteral("\\bfalse\\b"), QStringLiteral("\\bfinal\\b"), QStringLiteral("\\bfinally\\b"),
        QStringLiteral("\\bfloat\\b"), QStringLiteral("\\bfor\\b"), QStringLiteral("\\bfunction\\b"), QStringLiteral("\\bgoto\\b"),
        QStringLiteral("\\bif\\b"), QStringLiteral("\\bimplements\\b"), QStringLiteral("\\bin\\b"), QStringLiteral("\\binstanceof\\b"),
        QStringLiteral("\\bint\\b"),QStringLiteral("\\binterface\\b"), QStringLiteral("\\blet\\b"), QStringLiteral("\\blong\\b"),
        QStringLiteral("\\bnative\\b"), QStringLiteral("\\bnew\\b"), QStringLiteral("\\bnull\\b"), QStringLiteral("\\bpackage\\b"),
        QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"), QStringLiteral("\\breturn\\b"),
        QStringLiteral("\\bshort\\b"), QStringLiteral("\\bstatic\\b"), QStringLiteral("\\bswitch\\b"), QStringLiteral("\\bsynchronized\\b"),
        QStringLiteral("\\btry\\b"), QStringLiteral("\\btypeof\\b"), QStringLiteral("\\bvar\\b"), QStringLiteral("\\bvoid\\b"),
        QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\bwhile\\b"), QStringLiteral("\\bwith\\b"), QStringLiteral("\\byield\\b")
    };
    rule.nth = 0;
    for(const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = mFormatJSKeyword;
        mListJSHighlightRules << rule;
    }
}

void TextHighlighter::highlightBlock(const QString &text) {
    int highlightIndex = previousBlockState();
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
        highlight(highlightIndex, text, highlightStart, match.capturedStart() - highlightStart);
        highlightIndex = tagIndex(match.captured(1));
        highlightStart = match.capturedEnd();
    }

    //高亮尾部
    highlight(highlightIndex, text, highlightStart, text.length() - highlightStart);
    setCurrentBlockState(highlightIndex);
}

void TextHighlighter::highlightProduction(const QString &text, int start, int len) {
    int arrowStart = text.indexOf(' ', start);
    if(arrowStart == -1)
        return;
    arrowStart++;
    int arrowEnd = qMin(start + len, text.indexOf(' ', arrowStart));
    int arrowLen = (arrowEnd == -1 ? text.length(): arrowEnd) - arrowStart;
    const QTextCharFormat &format = (text.mid(arrowStart, arrowLen) == "->" ? mFormatProdArrow : mFormatProdWrongArrow);
    setFormat(arrowStart, arrowLen, format);
}

void TextHighlighter::highlightJS(const QString &text, int start, int len) {
    int end = start + len;
    for(const HighlightRule &rule : mListJSHighlightRules) {
        QRegularExpressionMatchIterator matchIter = rule.pattern.globalMatch(text, start);
        while(matchIter.hasNext()) {
            QRegularExpressionMatch match = matchIter.next();
            if(match.capturedEnd() > end)
                break;
            setFormat(match.capturedStart(rule.nth), match.capturedLength(rule.nth), rule.format);
        }
    }
}

void TextHighlighter::highlightOutput(const QString &text, int start, int len) {
    int end = start + len;
    //匹配
    QRegularExpressionMatchIterator matchIter = mRuleOutputFormat.globalMatch(text);
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

int TextHighlighter::tagIndex(const QString &tag) {
    auto iter = mMapTags.find(tag);
    return (iter == mMapTags.end() ? -1 : *iter);
}

void TextHighlighter::highlight(int index, const QString &text, int start, int len) {
    if(index < 0 || index >= arrFnHighlightLen)
        return;
    (this->*mArrFnHighlight[index])(text, start, len);
}
