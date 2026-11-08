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

    const QUrl absolute(href);
    if (absolute.isValid() && !absolute.scheme().isEmpty())
        return absolute;

    QUrl relative;
    relative.setPath(href);
    return base.resolved(relative);
}

// Scheme, host, port and path. The path stays, because a Nextcloud under a
// directory is only found by appending /remote.php/dav/ to that base.
QString serverBase(const QUrl &url)
{
    return url.toString(QUrl::FullyEncoded | QUrl::StripTrailingSlash
                        | QUrl::RemoveQuery | QUrl::RemoveFragment);
}

QString pathWithSlash(const QString &path)
{
    if (path.isEmpty() || path.endsWith(QLatin1Char('/')))
        return path;

    return path + QLatin1Char('/');
}

}
