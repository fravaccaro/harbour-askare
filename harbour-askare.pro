TARGET = harbour-askare

CONFIG += sailfishapp sailfishapp_i18n c++11
QT += dbus network sql

# sailfishapp.prf pulls in link_pkgconfig and resolves PKGCONFIG afterwards, so
# requesting link_pkgconfig here instead would drop -lsailfishapp from the link.
PKGCONFIG += sailfishsecrets

INCLUDEPATH += $$PWD/src

# Autocompletion for the Opal modules bundled under qml/modules. They install
# with the rest of the qml tree; main.cpp adds the matching runtime import path.
QML_IMPORT_PATH += qml/modules

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
    src/askare/reminders.cpp \
    src/askare/secretsstore.cpp \
    src/askare/session.cpp \
    src/askare/sharedtext.cpp \
    src/askare/smartlistmodel.cpp \
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
    src/askare/reminders.h \
    src/askare/secretsstore.h \
    src/askare/session.h \
    src/askare/sharedtext.h \
    src/askare/smartlistmodel.h \
    src/askare/store.h \
    src/askare/taskmodel.h \
    src/askare/types.h \
    src/askare/url.h

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

appicon.files = icons/appicon.png
appicon.path = /usr/share/$${TARGET}
INSTALLS += appicon

TRANSLATIONS += \
    translations/harbour-askare-en.ts \
    translations/harbour-askare-it.ts

DISTFILES += \
    harbour-askare.desktop \
    harbour-askare.desktop.in \
    scripts/desktop-translations.sh \
    translations/desktop/LINGUAS \
    translations/desktop/harbour-askare.pot \
    translations/desktop/it.po \
    icons/harbour-askare.svg \
    qml/harbour-askare.qml \
    qml/icons/icon-m-tag.svg \
    qml/components/AskareNotification.qml \
    qml/components/AskareSupportDialog.qml \
    qml/components/ConflictField.qml \
    qml/components/MultiSelectBox.qml \
    qml/components/MultiSelectChips.qml \
    qml/components/Selection.js \
    qml/components/ShareReceiver.qml \
    qml/components/TagChip.qml \
    qml/components/TaskListHeader.qml \
    qml/cover/CoverPage.qml \
    qml/cover/cover-check.png \
    qml/pages/AboutPage.qml \
    qml/pages/ConflictPage.qml \
    qml/pages/FilterItem.qml \
    qml/pages/IconField.qml \
    qml/pages/ListsPage.qml \
    qml/pages/LoginPage.qml \
    qml/pages/PriorityTag.qml \
    qml/pages/ReminderOptions.qml \
    qml/pages/SearchFilters.qml \
    qml/pages/SearchPage.qml \
    qml/pages/SettingsPage.qml \
    qml/pages/SmartListDialog.qml \
    qml/pages/StatusOptions.qml \
    qml/pages/SubtaskRow.qml \
    qml/pages/StartPage.qml \
    qml/pages/TagChips.qml \
    qml/pages/TaskListDelegate.qml \
    qml/pages/TagEditor.qml \
    qml/pages/TaskDetailPage.qml \
    qml/pages/TaskRow.qml \
    qml/pages/TaskEditPage.qml \
    qml/pages/TasksPage.qml \
    qml/pages/TranslationsPage.qml \
    qml/pages/WebLoginPage.qml \
    translations/harbour-askare.ts
