import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
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
    property string status: "NEEDS-ACTION"
    property string location
    property string url
    property var tags: []
    property bool listIndexReady: false
    // A task that names a parent. Moving an existing one to another list
    // detaches it, since RELATED-TO resolves by UID only within one calendar.
    readonly property bool isSubtask: page.parentUid !== ""
    // A subtask still being created stays under its parent; only an existing
    // one can be carried off and turned into a task of its own.
    readonly property bool newSubtask: page.isSubtask && page.href === ""

    function pad(value) {
        return value < 10 ? "0" + value : "" + value;
    }

    function asDate(iso) {
        if (iso === "")
            return new Date();

        var parts = iso.split("-");
        return new Date(Number(parts[0]), Number(parts[1]) - 1, Number(parts[2]));
    }

    // The pickers are the same for a start and a due moment, so both kinds of
    // field hand in what they hold and a function that takes the answer.
    function pickDate(iso, apply) {
        var dialog = pageStack.push("Sailfish.Silica.DatePickerDialog", {
            "date": page.asDate(iso)
        });
        dialog.accepted.connect(function() {
            apply(dialog.year + "-" + page.pad(dialog.month) + "-" + page.pad(dialog.day));
        });
    }

    function pickTime(clock, apply) {
        var parts = clock === "" ? ["9", "0"] : clock.split(":");
        var dialog = pageStack.push("Sailfish.Silica.TimePickerDialog", {
            "hour": Number(parts[0]),
            "minute": Number(parts[1])
        });
        dialog.accepted.connect(function() {
            apply(page.pad(dialog.hour) + ":" + page.pad(dialog.minute));
        });
    }

    function pickDue() {
        page.pickDate(page.dueDate, function(iso) {
            page.dueDate = iso;
        });
    }

    function pickDueTime() {
        page.pickTime(page.dueTime, function(clock) {
            page.dueTime = clock;
        });
    }

    function pickStart() {
        page.pickDate(page.startDate, function(iso) {
            page.startDate = iso;
        });
    }

    function pickStartTime() {
        page.pickTime(page.startTime, function(clock) {
            page.startTime = clock;
        });
    }

    function clearDue() {
        page.dueDate = "";
        page.dueTime = "";
        page.reminder = -1;
    }

    function clearStart() {
        page.startDate = "";
        page.startTime = "";
    }

    // Work cannot be due before it may begin. Both parts compare as text in
    // yyyy-MM-dd and HH:mm, and an all-day due accepts any time that day.
    function startAfterDue() {
        if (page.startDate === "" || page.dueDate === "")
            return false;

        if (page.startDate !== page.dueDate)
            return page.startDate > page.dueDate;

        if (page.dueTime === "")
            return false;

        return (page.startTime === "" ? "00:00" : page.startTime) > page.dueTime;
    }

    // A subtask being created stays where its parent is; anything else follows
    // the picker, which only has lists to offer once they have been read. A
    // task in a list the device does not know stays there too, since the picker
    // cannot show that list and would otherwise read as the first one.
    function chosenCalendar() {
        if (page.newSubtask || Calendars.count === 0)
            return page.calendarHref;

        if (page.href !== "" && Calendars.nameOfHref(page.calendarHref) === "")
            return page.calendarHref;

        var index = listBox.currentIndex < 0 ? 0 : listBox.currentIndex;
        return Calendars.hrefAt(index);
    }

    function applyListIndex() {
        if (page.listIndexReady || Calendars.count === 0)
            return ;

        listBox.currentIndex = Calendars.indexOfHref(page.calendarHref);
        page.listIndexReady = true;
    }

    // The tags in use name themselves; the counts beside them belong to the
    // lists page, so only the names are offered here.
    function tagNames() {
        var out = [];
        var all = Askare.tags;
        for (var i = 0; i < all.length; ++i) out.push(all[i].name)
        return out;
    }

    // Status and completion stay in step: reaching 100% marks the task done,
    // and dropping below it leaves a task that was done back in progress.
    function syncStatusToPercent() {
        if (!statusBox.ready)
            return ;

        if (page.percent >= 100)
            statusBox.currentIndex = statusOptions.indexOf("COMPLETED");
        else if (page.status === "COMPLETED")
            statusBox.currentIndex = statusOptions.indexOf(page.percent > 0 ? "IN-PROCESS" : "NEEDS-ACTION");
    }

    function save() {
        page.summary = summaryField.text;
        page.description = notesField.text;
        page.location = locationField.text;
        page.url = urlField.text;
        page.tags = tagEditor.tags;
        Askare.saveTask({
            "href": page.href,
            "calendarHref": page.chosenCalendar(),
            "parentUid": page.parentUid,
            "summary": page.summary,
            "description": page.description,
            "location": page.location,
            "url": page.url,
            "tags": page.tags,
            "startDate": page.startDate,
            "startTime": page.startTime,
            "dueDate": page.dueDate,
            "dueTime": page.dueTime,
            "priority": page.priority,
            "percent": page.percent,
            "status": page.status,
            "reminder": page.reminder
        });
    }

    // The window looks for this page by name, so a request to add a task from
    // the cover raises it instead of opening a second copy of it.
    objectName: "taskEditPage"
    allowedOrientations: Orientation.All
    canAccept: page.chosenCalendar() !== "" && summaryField.text.length > 0 && !page.startAfterDue()
    onAccepted: page.save()

    ReminderOptions {
        id: reminderOptions
    }

    StatusOptions {
        id: statusOptions
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: page.width

            DialogHeader {
                acceptText: page.href === "" ? qsTr("Add task") : qsTr("Save")
                title: page.newSubtask ? qsTr("New subtask") : (page.href === "" ? qsTr("New task") : qsTr("Task"))
            }

            TextField {
                id: summaryField

                width: parent.width
                text: page.summary
                label: qsTr("Name")
                placeholderText: qsTr("Name")
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: notesField.focus = true
            }

            ComboBox {
                id: listBox

                width: parent.width
                visible: !page.newSubtask
                label: qsTr("List")
                // Shown only once another list is picked, so an untouched
                // subtask does not warn about a move that is not happening.
                description: page.isSubtask && page.chosenCalendar() !== page.calendarHref ? qsTr("This subtask becomes a task of its own, and its own subtasks follow it.") : ""
                Component.onCompleted: page.applyListIndex()

                Connections {
                    target: Calendars
                    onCountChanged: page.applyListIndex()
                }

                menu: ContextMenu {
                    Repeater {
                        model: Calendars

                        MenuItem {
                            text: displayName
                        }

                    }

                }

            }

            ComboBox {
                id: statusBox

                property bool ready: false

                width: parent.width
                label: qsTr("Status")
                Component.onCompleted: {
                    currentIndex = statusOptions.indexOf(page.status);
                    ready = true;
                }
                onCurrentIndexChanged: {
                    if (!ready || currentIndex < 0)
                        return ;

                    page.status = statusOptions.values[currentIndex];
                    if (page.status === "COMPLETED")
                        completionSlider.value = 100;

                }

                menu: ContextMenu {
                    Repeater {
                        model: statusOptions.values

                        MenuItem {
                            text: statusOptions.labelFor(modelData)
                        }

                    }

                }

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

            Slider {
                id: completionSlider

                width: parent.width
                label: qsTr("Completion")
                minimumValue: 0
                maximumValue: 100
                stepSize: 5
                valueText: value + "%"
                Component.onCompleted: value = page.percent
                onValueChanged: {
                    page.percent = value;
                    page.syncStatusToPercent();
                }
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
                description: enabled ? qsTr("Send a notification while Askare is running.") : qsTr("Set a due date first.")
                Component.onCompleted: {
                    currentIndex = reminderOptions.indexOf(page.reminder);
                    ready = true;
                }
                onCurrentIndexChanged: {
                    if (ready && currentIndex >= 0)
                        page.reminder = reminderOptions.values[currentIndex];

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

            TextField {
                id: locationField

                width: parent.width
                text: page.location
                label: qsTr("Location")
                placeholderText: qsTr("Location")
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: urlField.focus = true
            }

            TextField {
                id: urlField

                width: parent.width
                text: page.url
                label: qsTr("Web address")
                placeholderText: qsTr("Web address")
                inputMethodHints: Qt.ImhUrlCharactersOnly | Qt.ImhNoAutoUppercase
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: notesField.focus = true
            }

            TagEditor {
                id: tagEditor

                width: parent.width
                tags: page.tags
                suggestions: page.tagNames()
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
