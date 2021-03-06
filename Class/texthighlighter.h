#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

#include "header.h"

class TextHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit TextHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
        int nth;
    };
    QList<HighlightRule> mListJSHighlightRules/*, mListOutputHighlightRules*/;

    QRegularExpression mRuleJSStringWithBracket = QRegularExpression("\\b([A-Za-z_]{1}[A-Za-z_0-9]*) *\\(");
    QRegularExpression mRuleJSQObj = QRegularExpression("\\blp\\b");
    QRegularExpression mRuleJSNumber = QRegularExpression("\\b(?:(?:[0-9]*\\.[0-9]+)|(?:[0-9]+))(?:e\\-?[0-9]+)?\\b");
    QRegularExpression mRuleJSHexNumber = QRegularExpression("\\b0x[0-9A-Fa-f]+\\b");
    QRegularExpression mRuleJSInnerHighlight = QRegularExpression("((?://)|(?:/\\*)|\"|'|(?:/[^*/]{1}))");
    QRegularExpression mRuleJSMultiLineCommitEnd = QRegularExpression("\\*/");
    QRegularExpression mRuleJSMqStringQuoteOrEnd = QRegularExpression("\\\\|\"");
    QRegularExpression mRuleJSSqStringQuoteOrEnd = QRegularExpression("\\\\|'");
    QRegularExpression mRuleTag = QRegularExpression("%\\[(.*?)(?:\\:(.*?))?\\]%");
    QRegularExpression mRuleOutputFormat = QRegularExpression("#\\[(.*?)(?:\\:(.*?))?\\]#");

    QTextCharFormat mFormatTagBracket;
    QTextCharFormat mFormatTagText;
    QTextCharFormat mFormatTagArg;

    QTextCharFormat mFormatOFBracket;
    QTextCharFormat mFormatOFText;
    QTextCharFormat mFormatOFArg;

    QTextCharFormat mFormatActionName;
    QTextCharFormat mFormatProdArrow;
    QTextCharFormat mFormatProdWrongArrow;

    QTextCharFormat mFormatJSStringWithBracket;
    QTextCharFormat mFormatJSKeyword;
    QTextCharFormat mFormatJSQObj;
    QTextCharFormat mFormatJSNumber;
    QTextCharFormat mFormatJSCommit;
    QTextCharFormat mFormatJSString;
    QTextCharFormat mFormatJSStrQuote;
    QTextCharFormat mFormatJSRegex;

    struct HighlightConfig;
    void highlightAction(HighlightConfig &hc);
    void highlightProduction(HighlightConfig &hc);
    void highlightJS(HighlightConfig &hc);
    void highlightOutput(HighlightConfig &hc);
    void highlightJSCommit(HighlightConfig &hc);
    void highlightJSMultiLineCommit(HighlightConfig &hc);
    void highlightJSString(HighlightConfig &hc, const QRegularExpression &regexQuoteOrEnd, QChar chEnd);
    void highlightJSMqString(HighlightConfig &hc) { highlightJSString(hc, mRuleJSMqStringQuoteOrEnd, '"'); }   //双引号字符串
    void highlightJSSqString(HighlightConfig &hc) { highlightJSString(hc, mRuleJSSqStringQuoteOrEnd, '\''); }  //单引号字符串
    void highlightJSRegex(HighlightConfig &hc);

    typedef void(TextHighlighter::*FnHighlight)(HighlightConfig &hc);
    QVector<FnHighlight> mVecFn;
    QMap<QString, int> mMapTags;

    QMap<QString, FnHighlight> mMapJSFn = {
        { "//", &TextHighlighter::highlightJSCommit },
        { "/*", &TextHighlighter::highlightJSMultiLineCommit },
        { "\"", &TextHighlighter::highlightJSMqString },
        { "'", &TextHighlighter::highlightJSSqString }
        //正则表达式使用if判断
    };

    struct HighlightConfig
    {
        HighlightConfig(const QString &text, FnHighlight fn) : text(text), fn(fn), prevFn(fn) {}
        void prepare(int _start, int _len) {
            start = _start;
            len = _len;
            offset = -1;
        }
        const QString &text;
        FnHighlight fn, prevFn;
        int start = -1;
        int len = -1;
        int offset = -1;
    };

    int tagIndex(const QString &tag) const;
    FnHighlight indexFn(int index) const;

    void highlight(HighlightConfig &hc);
};