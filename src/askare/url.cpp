#include "url.h"

namespace Url {

QString normalizeServer(const QString &input)
{
    QString url = input.trimmed();
    if (url.isEmpty())
        return QString();

    if (!url.contains(QStringLiteral("://")))
        url.prepend(QStringLiteral("https://"));

    while (url.endsWith(QLatin1Char('/')))
        url.chop(1);

    return url;
}

QUrl join(const QUrl &base, const QString &href)
{
    if (href.isEmpty())
        return base;

    return base.resolved(QUrl(href));
}

QString origin(const QUrl &url)
{
    QUrl root;
    root.setScheme(url.scheme());
    root.setHost(url.host());
    root.setPort(url.port());
    return root.toString();
}

QString pathWithSlash(const QString &path)
{
    if (path.isEmpty() || path.endsWith(QLatin1Char('/')))
        return path;

    return path + QLatin1Char('/');
}

}
