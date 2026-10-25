import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: page

    property string href
    property string calendarHref
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
    property bool listIndexReady: false

    // A subtask is pinned to its parent's list, since RELATED-TO resolves by
    // UID only within one calendar.
    readonly property bool isSubtask: page.parentUid !== ""

    function pad(value) {
        return value < 10 ? "0" + value : "" + value
    }

    function asDate(iso) {
        if (iso === "")
            return new Date()
        var parts = iso.split("-")
        return new Date(Number(parts[0]), Number(parts[1]) - 1, Number(parts[2]))
    }

    // The pickers are the same for a start and a due moment, so both kinds of
    // field hand in what they hold and a function that takes the answer.
    function pickDate(iso, apply) {
        var dialog = pageStack.push("Sailfish.Silica.DatePickerDialog", {
                                        "date": page.asDate(iso)
                                    })
        dialog.accepted.connect(function() {
            apply(dialog.year + "-" + page.pad(dialog.month) + "-" + page.pad(dialog.day))
        })
    }

    function pickTime(clock, apply) {
        var parts = clock === "" ? ["9", "0"] : clock.split(":")
        var dialog = pageStack.push("Sailfish.Silica.TimePickerDialog", {
                                        "hour": Number(parts[0]),
                                        "minute": Number(parts[1])
                                    })
        dialog.accepted.connect(function() {
            apply(page.pad(dialog.hour) + ":" + page.pad(dialog.minute))
        })
    }

    function pickDue() {
        page.pickDate(page.dueDate, function(iso) { page.dueDate = iso })
    }

    function pickDueTime() {
        page.pickTime(page.dueTime, function(clock) { page.dueTime = clock })
    }

    function pickStart() {
        page.pickDate(page.startDate, function(iso) { page.startDate = iso })
    }

    function pickStartTime() {
        page.pickTime(page.startTime, function(clock) { page.startTime = clock })
    }

    function clearDue() {
        page.dueDate = ""
        page.dueTime = ""
        page.reminder = -1
    }

    function clearStart() {
        page.startDate = ""
        page.startTime = ""
    }

    // Work cannot be due before it may begin. Both parts compare as text in
    // yyyy-MM-dd and HH:mm, and an all-day due accepts any time that day.
    function startAfterDue() {
        if (page.startDate === "" || page.dueDate === "")
            return false
        if (page.startDate !== page.dueDate)
            return page.startDate > page.dueDate
        if (page.dueTime === "")
            return false
        return (page.startTime === "" ? "00:00" : page.startTime) > page.dueTime
    }

    function chosenCalendar() {
        if (page.calendarHref !== "" && (page.href !== "" || page.isSubtask))
            return page.calendarHref
        var index = listBox.currentIndex < 0 ? 0 : listBox.currentIndex
        return Calendars.hrefAt(index)
    }

    function applyListIndex() {
        if (page.listIndexReady || Calendars.count === 0)
            return
        listBox.currentIndex = Calendars.indexOfHref(page.calendarHref)
        page.listIndexReady = true
    }

    function save() {
        page.summary = summaryField.text
        page.description = notesField.text
        Askare.saveTask({
                            "href": page.href,
                            "calendarHref": page.chosenCalendar(),
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
    }

    // The window looks for this page by name, so a request to add a task from
    // the cover raises it instead of opening a second copy of it.
    objectName: "taskEditPage"
    allowedOrientations: Orientation.All
    canAccept: page.chosenCalendar() !== "" && summaryField.text.length > 0
               && !page.startAfterDue()
    onAccepted: page.save()

    ReminderOptions {
        id: reminderOptions
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: page.width

            DialogHeader {
                acceptText: page.href === "" ? qsTr("Add task") : qsTr("Save")
                title: page.isSubtask ? qsTr("New subtask")
                                      : (page.href === "" ? qsTr("New task") : qsTr("Task"))
            }

            ComboBox {
                id: listBox

                width: parent.width
                visible: page.href === "" && !page.isSubtask
                label: qsTr("List")
                Component.onCompleted: page.applyListIndex()

                menu: ContextMenu {
                    Repeater {
                        model: Calendars
                        MenuItem {
                            text: displayName
                        }
                    }
                }

                Connections {
                    target: Calendars
                    onCountChanged: page.applyListIndex()
                }
            }

            DetailItem {
                visible: page.isSubtask
                label: qsTr("List")
                value: Calendars.nameOfHref(page.calendarHref)
            }

            TextField {
                id: summaryField

                width: parent.width
                text: page.summary
                label: qsTr("Title")
                placeholderText: qsTr("Title")
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: notesField.focus = true
            }

            Slider {
                width: parent.width
                label: qsTr("Completion")
                minimumValue: 0
                maximumValue: 100
                stepSize: 5
                valueText: value + "%"
                Component.onCompleted: value = page.percent
                onValueChanged: page.percent = value
            }

            PriorityTag {
                id: priorityBands

                visible: false
            }

            Slider {
                width: parent.width
                label: qsTr("Priority")
                minimumValue: 0
                maximumValue: 9
                stepSize: 1
                valueText: priorityBands.labelFor(value)
                Component.onCompleted: value = page.priority
                onValueChanged: page.priority = value
            }

            ValueButton {
                width: parent.width
                label: qsTr("Start")
                value: page.startDate === "" ? qsTr("None") : page.startDate
                onClicked: page.pickStart()
            }

            ValueButton {
                width: parent.width
                enabled: page.startDate !== ""
                label: qsTr("Start time")
                value: page.startTime === "" ? qsTr("All day") : page.startTime
                onClicked: page.pickStartTime()
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: page.startDate !== ""
                text: qsTr("Clear start date")
                onClicked: page.clearStart()
            }

            ValueButton {
                width: parent.width
                label: qsTr("Due")
                value: page.dueDate === "" ? qsTr("None") : page.dueDate
                onClicked: page.pickDue()
            }

            ValueButton {
                width: parent.width
                enabled: page.dueDate !== ""
                label: qsTr("Time")
                value: page.dueTime === "" ? qsTr("All day") : page.dueTime
                onClicked: page.pickDueTime()
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                visible: page.startAfterDue()
                color: Theme.errorColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("The start must not be after the due date.")
            }

            ComboBox {
                // Guards the task's reminder while the menu is still building.
                property bool ready: false

                width: parent.width
                enabled: page.dueDate !== ""
                label: qsTr("Reminder")
                description: enabled ? qsTr("Alerts while Askare is running.")
                                     : qsTr("Set a due date first.")
                Component.onCompleted: {
                    currentIndex = reminderOptions.indexOf(page.reminder)
                    ready = true
                }
                onCurrentIndexChanged: {
                    if (ready && currentIndex >= 0)
                        page.reminder = reminderOptions.values[currentIndex]
                }

                menu: ContextMenu {
                    Repeater {
                        model: reminderOptions.values
                        MenuItem {
                            text: reminderOptions.labelFor(modelData)
                        }
                    }
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: page.dueDate !== ""
                text: qsTr("Clear due date")
                onClicked: page.clearDue()
            }

            TextArea {
                id: notesField

                width: parent.width
                text: page.description
                label: qsTr("Notes")
                placeholderText: qsTr("Notes")
            }
        }

        VerticalScrollDecorator {
        }
    }
}
