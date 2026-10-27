import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property string href
    property string calendarHref
    property string uid
    property string parentUid
    property string summary
    property string description
    property string startDate
    property string startTime
    property string dueDate
    property string dueTime
    property int priority: 0
    property int percent: 0
    property int reminder: -1

    property var subtasks: []

    readonly property bool isDone: page.percent >= 100
    readonly property int subtaskDoneCount: page.countDone(page.subtasks)

    // One moment in words, for a start, a due date, or a subtask's own.
    function moment(date, time) {
        if (date === "")
            return ""
        return time === "" ? date : date + " " + time
    }

    // The same moment for a row that stays put when the task has none.
    function momentOr(date, time) {
        return date === "" ? qsTr("None") : page.moment(date, time)
    }

    function countDone(list) {
        var done = 0
        for (var i = 0; i < list.length; ++i) {
            if (list[i].done)
                ++done
        }
        return done
    }

    function reloadSubtasks() {
        page.subtasks = page.uid === "" ? [] : Askare.subtasks(page.uid)
    }

    function addSubtask() {
        pageStack.push(Qt.resolvedUrl("TaskEditPage.qml"), {
                           "calendarHref": page.calendarHref,
                           "parentUid": page.uid
                       })
    }

    function applyEdit(dialog) {
        page.summary = dialog.summary
        page.description = dialog.description
        page.startDate = dialog.startDate
        page.startTime = dialog.startTime
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
                                        "parentUid": page.parentUid,
                                        "summary": page.summary,
                                        "description": page.description,
                                        "startDate": page.startDate,
                                        "startTime": page.startTime,
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
    Component.onCompleted: page.reloadSubtasks()

    ReminderOptions {
        id: reminderOptions
    }

    // Instantiated only for its band names, so the mapping stays in one place.
    PriorityTag {
        id: priorityBands

        visible: false
    }

    Connections {
        target: Askare
        onTasksChanged: page.reloadSubtasks()
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("Delete")
                onClicked: page.remove()
            }
            MenuItem {
                text: qsTr("Add subtask")
                onClicked: page.addSubtask()
            }
            MenuItem {
                text: qsTr("Edit")
                onClicked: page.edit()
            }
            MenuItem {
                text: page.isDone ? qsTr("Mark to-do") : qsTr("Mark done")
                onClicked: page.toggleDone()
            }
        }

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: page.summary
            }

            DetailItem {
                label: qsTr("Priority")
                value: priorityBands.labelFor(page.priority)
            }

            DetailItem {
                label: qsTr("Start")
                value: page.momentOr(page.startDate, page.startTime)
            }

            DetailItem {
                label: qsTr("Due")
                value: page.momentOr(page.dueDate, page.dueTime)
            }

            DetailItem {
                label: qsTr("Reminder")
                value: reminderOptions.labelFor(page.reminder)
            }

            ProgressBar {
                width: parent.width
                minimumValue: 0
                maximumValue: 100
                value: page.percent
                label: qsTr("Completion")
                valueText: page.percent + "%"
            }

            SectionHeader {
                text: qsTr("Notes")
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

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                visible: page.description === ""
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("None")
            }

            SectionHeader {
                visible: page.subtasks.length > 0
                text: qsTr("Subtasks")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                visible: page.subtasks.length > 0
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("%1 of %2 done").arg(page.subtaskDoneCount).arg(page.subtasks.length)
            }

            Repeater {
                model: page.subtasks

                SubtaskRow {
                    width: page.width
                    summary: modelData.summary
                    dueText: page.moment(modelData.dueDate, modelData.dueTime)
                    done: modelData.done
                    onToggled: Askare.toggleDone(modelData.href)
                    onClicked: app.pushTask(modelData.href)
                }
            }
        }

        VerticalScrollDecorator {
        }
    }
}
