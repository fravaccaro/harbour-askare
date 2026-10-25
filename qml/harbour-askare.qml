import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Notifications 1.0
import "pages"

ApplicationWindow {
    id: app

    function showMain() {
        if (pageStack.busy) {
            navigateAgain.restart()
            return
        }
        pageStack.replaceAbove(null, Askare.configured ? tasksPage : loginPage)
    }

    initialPage: Askare.ready ? (Askare.configured ? tasksPage : loginPage) : startPage
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    Timer {
        id: navigateAgain

        interval: 100
        onTriggered: app.showMain()
    }

    Component {
        id: startPage

        StartPage {
        }
    }

    Component {
        id: loginPage

        LoginPage {
        }
    }

    Component {
        id: tasksPage

        TasksPage {
        }
    }

    Connections {
        target: Askare
        onReadyChanged: {
            if (Askare.ready)
                app.showMain()
        }
        onConfiguredChanged: {
            if (!Askare.configured)
                app.showMain()
        }
    }

    Notification {
        id: reminderNotification

        appName: "Askare"
        icon: "image://theme/icon-lock-information"
        category: "x-nemo.generic"
    }

    Connections {
        target: Reminders
        onDue: {
            reminderNotification.summary = summary
            reminderNotification.body = dueText === ""
                    ? qsTr("Task reminder") : qsTr("Due %1").arg(dueText)
            reminderNotification.previewSummary = reminderNotification.summary
            reminderNotification.previewBody = reminderNotification.body
            reminderNotification.replacesId = 0
            reminderNotification.publish()
        }
    }
}
