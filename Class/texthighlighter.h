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

    QRegularExpression mRuleJSStringWithBracket = QRegularExpression("\\b([A-Za-z]+) *\\(");
    QRegularExpression mRuleJSQObj = QRegularExpression("\\blp\\b");
    QRegularExpression mRuleJSNumber = QRegularExpression("\\b[0-9]*(?:\\.[0-9]+){0,1}(?:e\\-{0,1}[0-9]+){0,1}\\b");
    QRegularExpression mRuleJSHexNumber = QRegularExpression("\\b0x[0-9A-Fa-f]+\\b");
    QRegularExpression mRuleTag = QRegularExpression("%\\[(.*?)(?:\\:(.*?)){0,1}\\]%");
    QRegularExpression mRuleOutputFormat = QRegularExpression("#\\[(.*?)(?:\\:(.*?)){0,1}\\]#");

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

    void highlightProduction(const QString &text, int start, int len);
    void highlightJS(const QString &text, int start, int len);
    void highlightOutput(const QString &text, int start, int len);

    typedef void(TextHighlighter::*FnHighlight)(const QString &text, int start, int len);
    static constexpr int arrFnHighlightLen = 3;
    const FnHighlight mArrFnHighlight[arrFnHighlightLen] = {
        &TextHighlighter::highlightProduction,
        &TextHighlighter::highlightJS,
        &TextHighlighter::highlightOutput
    };

    const QMap<QString, int> mMapTags = {
        { "Production", 0 },
        { "JS", 1 },
        { "Output", 2 }
    };

    int tagIndex(const QString &tag);
    void highlight(int index, const QString &text, int start, int len);
};