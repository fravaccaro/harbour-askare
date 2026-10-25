TARGET = harbour-askare

CONFIG += sailfishapp sailfishapp_i18n c++11
QT += network sql

# sailfishapp.prf pulls in link_pkgconfig and resolves PKGCONFIG afterwards, so
# requesting link_pkgconfig here instead would drop -lsailfishapp from the link.
PKGCONFIG += sailfishsecrets

INCLUDEPATH += $$PWD/src

isEmpty(APP_VERSION): APP_VERSION = 0.0-devel
DEFINES += APP_VERSION=\\\"$$APP_VERSION\\\"

CONFIG(debug, debug|release): DEFINES += QT_QML_DEBUG

SOURCES += \
    src/main.cpp \
    src/askare/calendarmodel.cpp \
    src/askare/config.cpp \
    src/askare/davclient.cpp \
    src/askare/davxml.cpp \
    src/askare/httpclient.cpp \
    src/askare/ics.cpp \
    src/askare/loginflow.cpp \
    src/askare/markdown.cpp \
    src/askare/secretsstore.cpp \
    src/askare/session.cpp \
    src/askare/store.cpp \
    src/askare/taskmodel.cpp \
    src/askare/url.cpp

HEADERS += \
    src/askare/calendarmodel.h \
    src/askare/config.h \
    src/askare/davclient.h \
    src/askare/davxml.h \
    src/askare/httpclient.h \
    src/askare/ics.h \
    src/askare/loginflow.h \
    src/askare/markdown.h \
    src/askare/secretsstore.h \
    src/askare/session.h \
    src/askare/store.h \
    src/askare/taskmodel.h \
    src/askare/types.h \
    src/askare/url.h

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

appicon.files = icons/appicon.png
appicon.path = /usr/share/$${TARGET}
INSTALLS += appicon

TRANSLATIONS += \
    translations/harbour-askare-it.ts

DISTFILES += \
    harbour-askare.desktop \
    qml/harbour-askare.qml \
    qml/cover/CoverPage.qml \
    qml/pages/AboutPage.qml \
    qml/pages/FilterItem.qml \
    qml/pages/ListsPage.qml \
    qml/pages/LoginPage.qml \
    qml/pages/PriorityTag.qml \
    qml/pages/SettingsPage.qml \
    qml/pages/StartPage.qml \
    qml/pages/TaskDetailPage.qml \
    qml/pages/TaskEditPage.qml \
    qml/pages/TasksPage.qml \
    qml/pages/WebLoginPage.qml \
    translations/harbour-askare.ts
