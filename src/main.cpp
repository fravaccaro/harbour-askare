#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QGuiApplication>
#include <QLocale>
#include <QQmlContext>
#include <QQuickView>
#include <QScopedPointer>
#include <QTranslator>

#include <sailfishapp.h>

#include "askare/calendarmodel.h"
#include "askare/config.h"
#include "askare/session.h"
#include "askare/store.h"
#include "askare/taskmodel.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    app->setOrganizationName(QStringLiteral("org.frapps"));
    app->setOrganizationDomain(QStringLiteral("frapps.org"));
    app->setApplicationName(QStringLiteral("harbour-askare"));
    app->setApplicationVersion(QStringLiteral(APP_VERSION));

    QScopedPointer<QTranslator> translator(new QTranslator);
    if (translator->load(QLocale(), QStringLiteral("harbour-askare"), QStringLiteral("-"),
                         SailfishApp::pathTo(QStringLiteral("translations")).toLocalFile())) {
        app->installTranslator(translator.data());
    }

    Config config;
    Store store;
    CalendarModel calendars(&store);
    TaskModel tasks(&store);
    Session session(&config, &store, &calendars, &tasks);

    QScopedPointer<QQuickView> view(SailfishApp::createView());
    QQmlContext *context = view->rootContext();
    context->setContextProperty(QStringLiteral("Askare"), &session);
    context->setContextProperty(QStringLiteral("Calendars"), &calendars);
    context->setContextProperty(QStringLiteral("Tasks"), &tasks);

    view->setSource(SailfishApp::pathTo(QStringLiteral("qml/harbour-askare.qml")));
    view->show();
    return app->exec();
}
