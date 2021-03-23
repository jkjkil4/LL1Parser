#include "texthighlighter.h"

#include <QDebug>

TextHighlighter::TextHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    mFormatTagBracket.setForeground(Qt::blue);
    mFormatTagText.setForeground(Qt::magenta);
    mFormatTagArg.setForeground(Qt::darkMagenta);
    mFormatTagArg.setFontWeight(QFont::Bold);
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
    setFormat(start, len, Qt::red);
}

void TextHighlighter::highlightJS(const QString &text, int start, int len) {
    setFormat(start, len, Qt::blue);
}

int TextHighlighter::tagIndex(const QString &tag) {
    auto iter = mapTags.find(tag);
    return (iter == mapTags.end() ? -1 : *iter);
}

void TextHighlighter::highlight(int index, const QString &text, int start, int len) {
    if(index < 0 || index >= arrFnHighlightLen)
        return;
    (this->*arrFnHighlight[index])(text, start, len);
}
