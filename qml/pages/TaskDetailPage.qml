import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property string href
    property string calendarHref
    property string summary
    property string description
    property string dueDate
    property string dueTime
    property int priority: 0
    property int percent: 0
    property int reminder: -1

    readonly property bool isDone: page.percent >= 100

    function dueText() {
        if (page.dueDate === "")
            return ""
        return page.dueTime === "" ? page.dueDate : page.dueDate + " " + page.dueTime
    }

    function applyEdit(dialog) {
        page.summary = dialog.summary
        page.description = dialog.description
        page.dueDate = dialog.dueDate
        page.dueTime = dialog.dueTime
        page.priority = dialog.priority
        page.percent = dialog.percent
        page.reminder = dialog.reminder
    }

    function edit() {
        var dialog = pageStack.push(Qt.resolvedUrl("TaskEditPage.qml"), {
                                        "href": page.href,
                                        "calendarHref": page.calendarHref,
                                        "summary": page.summary,
                                        "description": page.description,
                                        "dueDate": page.dueDate,
                                        "dueTime": page.dueTime,
                                        "priority": page.priority,
                                        "percent": page.percent,
                                        "reminder": page.reminder
                                    })
        dialog.accepted.connect(function() { page.applyEdit(dialog) })
    }

    function toggleDone() {
        Askare.toggleDone(page.href)
        page.percent = page.isDone ? 0 : 100
    }

    function remove() {
        Remorse.popupAction(page, qsTr("Deleting"), function() {
            Askare.deleteTask(page.href)
            pageStack.pop()
        })
    }

    allowedOrientations: Orientation.All

    ReminderOptions {
        id: reminderOptions
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("Edit")
                onClicked: page.edit()
            }
            MenuItem {
                text: page.isDone ? qsTr("Mark open") : qsTr("Mark done")
                onClicked: page.toggleDone()
            }
            MenuItem {
                text: qsTr("Delete")
                onClicked: page.remove()
            }
        }

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: page.summary
            }

            PriorityTag {
                x: Theme.horizontalPageMargin
                priority: page.priority
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                visible: page.dueDate !== ""
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Due %1").arg(page.dueText())
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                visible: page.reminder >= 0
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: reminderOptions.labelFor(page.reminder)
            }

            ProgressBar {
                width: parent.width
                minimumValue: 0
                maximumValue: 100
                value: page.percent
                label: qsTr("Completion")
                valueText: page.percent + "%"
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                visible: page.description !== ""
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                color: Theme.primaryColor
                linkColor: Theme.highlightColor
                text: Askare.renderNotes(page.description)
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        VerticalScrollDecorator {
        }
    }
}
