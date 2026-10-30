import QtQuick 2.0
import Sailfish.Silica 1.0
import Opal.LinkHandler 1.0

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
    property string status: "NEEDS-ACTION"
    property string location
    property string url
    property var tags: []
    property var subtasks: []
    readonly property bool isDone: page.percent >= 100
    readonly property int subtaskDoneCount: page.countDone(page.subtasks)

    // One moment in words, for a start, a due date, or a subtask's own.
    function moment(date, time) {
        if (date === "")
            return "";

        return time === "" ? date : date + " " + time;
    }

    function countDone(list) {
        var done = 0;
        for (var i = 0; i < list.length; ++i) {
            if (list[i].done)
                ++done;

        }
        return done;
    }

    function reloadSubtasks() {
        page.subtasks = page.uid === "" ? [] : Askare.subtasks(page.uid);
    }

    function addSubtask() {
        pageStack.push(Qt.resolvedUrl("TaskEditPage.qml"), {
            "calendarHref": page.calendarHref,
            "parentUid": page.uid
        });
    }

    function applyEdit(dialog) {
        page.summary = dialog.summary;
        page.description = dialog.description;
        page.startDate = dialog.startDate;
        page.startTime = dialog.startTime;
        page.dueDate = dialog.dueDate;
        page.dueTime = dialog.dueTime;
        page.priority = dialog.priority;
        page.percent = dialog.percent;
        page.reminder = dialog.reminder;
        page.status = dialog.status;
        page.location = dialog.location;
        page.url = dialog.url;
        page.tags = dialog.tags;
    }

    function edit() {
        var dialog = pageStack.push(Qt.resolvedUrl("TaskEditPage.qml"), {
            "href": page.href,
            "calendarHref": page.calendarHref,
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
        dialog.accepted.connect(function() {
            page.applyEdit(dialog);
        });
    }

    function toggleDone() {
        Askare.toggleDone(page.href);
        page.percent = page.isDone ? 0 : 100;
    }

    function remove() {
        Remorse.popupAction(page, qsTr("Deleting"), function() {
            Askare.deleteTask(page.href);
            pageStack.pop();
        });
    }

    function followMove(newHref) {
        var fields = Askare.taskFields(newHref);
        page.href = newHref;
        page.calendarHref = fields.calendarHref;
        // A moved subtask is detached, so the page must forget its old parent
        // or a later edit would re-attach a link into the list it left.
        page.parentUid = fields.parentUid;
    }

    allowedOrientations: Orientation.All
    Component.onCompleted: page.reloadSubtasks()

    ReminderOptions {
        id: reminderOptions
    }

    StatusOptions {
        id: statusOptions
    }

    // Instantiated only for its band names, so the mapping stays in one place.
    PriorityTag {
        id: priorityBands

        visible: false
    }

    Connections {
        target: Askare
        onTasksChanged: page.reloadSubtasks()
        // A task carried to another list lives at a new href; without this the
        // page would keep acting on the one left behind.
        onTaskMoved: {
            if (oldHref === page.href)
                page.followMove(newHref);

        }
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
                description: Calendars.nameOfHref(page.calendarHref)
            }

            IconMenuItem {
                description: qsTr("Status")
                text: statusOptions.labelFor(page.status)
                icon.source: statusOptions.iconFor(page.status)
            }

            IconMenuItem {
                description: qsTr("Priority")
                visible: page.priority > 0
                text: priorityBands.labelFor(page.priority)
                icon.source: priorityBands.iconFor(page.priority)
            }

            IconMenuItem {
                description: qsTr("Start")
                visible: page.startDate !== ""
                text: page.moment(page.startDate, page.startTime)
                icon.source: "image://theme/icon-m-date"
            }

            IconMenuItem {
                description: qsTr("Due")
                visible: page.dueDate !== ""
                text: page.moment(page.dueDate, page.dueTime)
                icon.source: "image://theme/icon-m-date"
            }

            IconMenuItem {
                description: qsTr("Reminder")
                visible: page.reminder >= 0
                text: reminderOptions.labelFor(page.reminder)
                icon.source: "image://theme/icon-m-alarm"
            }

            IconMenuItem {
                description: qsTr("Location")
                visible: page.location !== ""
                text: page.location
                icon.source: "image://theme/icon-m-location"
            }

            // A MenuItem only emits clicked inside a menu, so the row is a
            // BackgroundItem: it gives the tap highlight and feeds it into the
            // IconMenuItem so the icon and text light up with it.
            BackgroundItem {
                id: urlRow

                width: parent.width
                contentHeight: urlItem.height
                visible: page.url !== ""
                onClicked: LinkHandler.openOrCopyUrl(page.url, page.summary)

                IconMenuItem {
                    id: urlItem

                    highlighted: urlRow.highlighted
                    description: qsTr("Website")
                    text: page.url
                    icon.source: "image://theme/icon-m-website"
                }
            }
            SectionHeader {
                text: qsTr("Completion")
            }

            ProgressBar {
                width: parent.width
                minimumValue: 0
                maximumValue: 100
                value: page.percent
                valueText: page.percent + "%"
            }

             
            SectionHeader {
                text: qsTr("Tags")
            }

            TagChips {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                tags: page.tags
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
                    cancelled: modelData.cancelled
                    subtreeCount: Askare.subtreeSize(modelData.href)
                    onToggled: Askare.toggleDone(modelData.href)
                    onEditRequested: app.editTask(modelData.href)
                    onDeleteConfirmed: Askare.deleteTask(modelData.href)
                    onClicked: app.pushTask(modelData.href)
                }

            }

        }

        VerticalScrollDecorator {
        }

    }

}
