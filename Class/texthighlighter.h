#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

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
    QRegularExpression mRuleJSNumber = QRegularExpression("\\b(?:(?:[0-9]*\\.[0-9]+)|(?:[0-9+]))(?:e\\-?[0-9]+)?\\b");
    QRegularExpression mRuleJSHexNumber = QRegularExpression("\\b0x[0-9A-Fa-f]+\\b");
    QRegularExpression mRuleJSInnerHighlight = QRegularExpression("((?://)|(?:/\\*)|\")");
    QRegularExpression mRuleJSMultiLineCommitEnd = QRegularExpression("\\*/");
    QRegularExpression mRuleJSStringQuoteOrEnd = QRegularExpression("\\\\|\"");
    QRegularExpression mRuleTag = QRegularExpression("%\\[(.*?)(?:\\:(.*?))?\\]%");
    QRegularExpression mRuleOutputFormat = QRegularExpression("#\\[(.*?)(?:\\:(.*?))?\\]#");

    QTextCharFormat mFormatTagBracket;
    QTextCharFormat mFormatTagText;
    QTextCharFormat mFormatTagArg;

    QTextCharFormat mFormatOFBracket;
    QTextCharFormat mFormatOFText;
    QTextCharFormat mFormatOFArg;

    QTextCharFormat mFormatProdArrow;
    QTextCharFormat mFormatProdWrongArrow;

    QTextCharFormat mFormatJSStringWithBracket;
    QTextCharFormat mFormatJSKeyword;
    QTextCharFormat mFormatJSQObj;
    QTextCharFormat mFormatJSNumber;
    QTextCharFormat mFormatJSCommit;
    QTextCharFormat mFormatJSString;
    QTextCharFormat mFormatJSStrQuote;

    struct HighlightConfig;
    void highlightProduction(HighlightConfig &hc);
    void highlightJS(HighlightConfig &hc);
    void highlightOutput(HighlightConfig &hc);
    void highlightJSCommit(HighlightConfig &hc);
    void highlightJSMultiLineCommit(HighlightConfig &hc);
    void highlightJSString(HighlightConfig &hc);

    typedef void(TextHighlighter::*FnHighlight)(HighlightConfig &hc);
    QVector<FnHighlight> mVecFn;
    QMap<QString, int> mMapTags;

    QMap<QString, FnHighlight> mMapJSFn = {
        { "//", &TextHighlighter::highlightJSCommit },
        { "/*", &TextHighlighter::highlightJSMultiLineCommit },
        { "\"", &TextHighlighter::highlightJSString }
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

    int tagIndex(const QString &tag);
    FnHighlight indexFn(int index);

    void highlight(HighlightConfig &hc);
};