#include "sharedtext.h"

#include <QUrl>

namespace {

// Text arrives from whichever app sent it, so the line endings are made into
// the ones a task is written and stored with, and the characters no editor
// would have let the user type are dropped.
QString cleaned(const QString &text)
{
    QString out = text;
    out.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    out.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    out.remove(QChar(QChar::Null));
    return out.trimmed();
}

// An address worth keeping is one a tap on the task could have followed. A
// file path is not: the app reads no directory of the device, so a task
// holding one would promise something it cannot do.
bool followable(const QUrl &url)
{
    const QString scheme = url.scheme().toLower();
    return url.isValid()
            && (scheme == QStringLiteral("http")
                || scheme == QStringLiteral("https")
                || scheme == QStringLiteral("mailto"));
}

}

QString SharedText::body(const QString &text)
{
    return cleaned(text);
}

QString SharedText::firstLine(const QString &text)
{
    const QString out = cleaned(text);
    const int breakAt = out.indexOf(QLatin1Char('\n'));
    return breakAt < 0 ? out : out.left(breakAt).trimmed();
}

QString SharedText::address(const QString &url)
{
    const QString out = cleaned(url);
    return followable(QUrl(out)) ? out : QString();
}
