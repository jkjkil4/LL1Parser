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
    QRegularExpression mRuleTag = QRegularExpression("%\\[(.*?)(?:\\:(.*?)){0,1}\\]%");

    QTextCharFormat mFormatTagBracket;
    QTextCharFormat mFormatTagText;
    QTextCharFormat mFormatTagArg;


    void highlightProduction(const QString &text, int start, int len);
    void highlightJS(const QString &text, int start, int len);

    typedef void(TextHighlighter::*FnHighlight)(const QString &text, int start, int len);
    static constexpr int arrFnHighlightLen = 2;
    const FnHighlight arrFnHighlight[arrFnHighlightLen] = {
        &TextHighlighter::highlightProduction,
        &TextHighlighter::highlightJS
    };

    const QMap<QString, int> mapTags = {
        { "Production", 0 },
        { "JS", 1 }
    };

    int tagIndex(const QString &tag);
    void highlight(int index, const QString &text, int start, int len);
};