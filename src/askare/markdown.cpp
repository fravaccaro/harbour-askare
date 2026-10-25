#include "markdown.h"

#include <QRegularExpression>
#include <QStringList>

namespace {

QString escaped(const QString &text)
{
    QString out = text;
    out.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    out.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    out.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    return out;
}

QString withInline(const QString &text)
{
    QString out = text;
    out.replace(QRegularExpression(QStringLiteral("`([^`]+)`")),
                QStringLiteral("<code>\\1</code>"));
    out.replace(QRegularExpression(QStringLiteral("\\[([^\\]]+)\\]\\(([^)\\s]+)\\)")),
                QStringLiteral("<a href=\"\\2\">\\1</a>"));
    out.replace(QRegularExpression(QStringLiteral("\\*\\*([^*]+)\\*\\*")),
                QStringLiteral("<b>\\1</b>"));
    out.replace(QRegularExpression(QStringLiteral("__([^_]+)__")),
                QStringLiteral("<b>\\1</b>"));
    out.replace(QRegularExpression(QStringLiteral("(?<![*\\w])\\*([^*]+)\\*(?!\\w)")),
                QStringLiteral("<i>\\1</i>"));
    out.replace(QRegularExpression(QStringLiteral("(?<![_\\w])_([^_]+)_(?!\\w)")),
                QStringLiteral("<i>\\1</i>"));
    return out;
}

int headingLevel(const QString &line)
{
    int level = 0;
    while (level < line.size() && level < 6 && line.at(level) == QLatin1Char('#'))
        ++level;
    if (level == 0 || level >= line.size() || line.at(level) != QLatin1Char(' '))
        return 0;
    return level;
}

bool isBullet(const QString &line)
{
    return line.startsWith(QLatin1String("- ")) || line.startsWith(QLatin1String("* "))
            || line.startsWith(QLatin1String("+ "));
}

QString orderedBody(const QString &line)
{
    const QRegularExpressionMatch match =
            QRegularExpression(QStringLiteral("^\\d+\\.\\s+(.*)$")).match(line);
    return match.hasMatch() ? match.captured(1) : QString();
}

bool isRule(const QString &line)
{
    return line == QLatin1String("---") || line == QLatin1String("***")
            || line == QLatin1String("___");
}

struct Builder
{
    QStringList out;
    QStringList para;
    QString listTag;
    bool quoting;

    Builder()
        : quoting(false)
    {
    }

    void flushParagraph()
    {
        if (para.isEmpty())
            return;
        out.append(QStringLiteral("<p>") + withInline(para.join(QStringLiteral("<br/>")))
                   + QStringLiteral("</p>"));
        para.clear();
    }

    void closeList()
    {
        if (listTag.isEmpty())
            return;
        out.append(QStringLiteral("</") + listTag + QStringLiteral(">"));
        listTag.clear();
    }

    void openList(const QString &tag)
    {
        if (listTag == tag)
            return;
        closeList();
        out.append(QStringLiteral("<") + tag + QStringLiteral(">"));
        listTag = tag;
    }

    void closeQuote()
    {
        if (!quoting)
            return;
        out.append(QStringLiteral("</blockquote>"));
        quoting = false;
    }

    void openQuote()
    {
        if (quoting)
            return;
        out.append(QStringLiteral("<blockquote>"));
        quoting = true;
    }

    void closeAll()
    {
        flushParagraph();
        closeList();
        closeQuote();
    }
};

void appendHeading(Builder *builder, const QString &line, int level)
{
    builder->closeAll();
    const QString tag = QStringLiteral("h%1").arg(level);
    builder->out.append(QStringLiteral("<") + tag + QStringLiteral(">")
                        + withInline(escaped(line.mid(level + 1).trimmed()))
                        + QStringLiteral("</") + tag + QStringLiteral(">"));
}

void appendItem(Builder *builder, const QString &tag, const QString &body)
{
    builder->flushParagraph();
    builder->closeQuote();
    builder->openList(tag);
    builder->out.append(QStringLiteral("<li>") + withInline(escaped(body))
                        + QStringLiteral("</li>"));
}

void appendQuote(Builder *builder, const QString &body)
{
    builder->flushParagraph();
    builder->closeList();
    builder->openQuote();
    builder->out.append(withInline(escaped(body)) + QStringLiteral("<br/>"));
}

bool appendListOrQuote(Builder *builder, const QString &line)
{
    if (isBullet(line)) {
        appendItem(builder, QStringLiteral("ul"), line.mid(2));
        return true;
    }
    const QString ordered = orderedBody(line);
    if (!ordered.isEmpty()) {
        appendItem(builder, QStringLiteral("ol"), ordered);
        return true;
    }
    if (line.startsWith(QLatin1String("> "))) {
        appendQuote(builder, line.mid(2));
        return true;
    }
    return false;
}

void appendLine(Builder *builder, const QString &raw)
{
    const QString line = raw.trimmed();
    if (line.isEmpty()) {
        builder->closeAll();
        return;
    }
    if (isRule(line)) {
        builder->closeAll();
        builder->out.append(QStringLiteral("<hr/>"));
        return;
    }
    const int level = headingLevel(line);
    if (level > 0) {
        appendHeading(builder, line, level);
        return;
    }
    if (appendListOrQuote(builder, line))
        return;

    builder->closeList();
    builder->closeQuote();
    builder->para.append(escaped(line));
}

}

namespace Markdown {

QString toHtml(const QString &source)
{
    if (source.trimmed().isEmpty())
        return QString();

    QString text = source;
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    Builder builder;
    const QStringList lines = text.split(QLatin1Char('\n'));
    for (const QString &line : lines)
        appendLine(&builder, line);
    builder.closeAll();

    return builder.out.join(QLatin1Char('\n'));
}

}
