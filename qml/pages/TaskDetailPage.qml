import Opal.LinkHandler 1.0
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

    // The page is a view of the stored task rather than a copy of it, so an
    // edit, a toggle and a sync all reach it the same way. The properties are
    // named after the keys taskFields() returns, which is what pushTask()
    // builds the page from in the first place.
    function reload() {
        var fields = Askare.taskFields(page.href);
        if (!fields.href)
            return ;

        for (var key in fields) page[key] = fields[key]
        page.reloadSubtasks();
    }

    function addSubtask() {
        pageStack.push(Qt.resolvedUrl("TaskEditPage.qml"), {
            "calendarHref": page.calendarHref,
            "parentUid": page.uid
        });
    }

    // The editor reads the task from the store, the same as a list row's own
    // Edit does, and reload() brings the answer back.
    function edit() {
        app.editTask(page.href);
    }

    function toggleDone() {
        Askare.toggleDone(page.href);
    }

    function remove() {
        Remorse.popupAction(page, qsTr("Deleting"), function() {
            Askare.deleteTask(page.href);
            pageStack.pop();
        });
    }

    // A moved subtask is detached, so reload() is what makes the page forget
    // the parent it left behind along with the list it came from.
    function followMove(newHref) {
        page.href = newHref;
        page.reload();
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
        onTasksChanged: page.reload()
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
                // A long title wraps rather than fading out; the header grows
                // with it, and the page scrolls.
                wrapMode: Text.WordWrap
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

            IconField {
                id: completionField

                width: parent.width
                description: qsTr("Completion")
                icon.source: "image://theme/icon-m-acknowledge"
                lineCenterY: completionBar.barCenterY
                lineBottomY: lineCenterY

                ProgressBar {
                    id: completionBar

                    width: parent.width
                    leftMargin: completionField.grooveLeftMargin
                    rightMargin: completionField.grooveRightMargin
                    minimumValue: 0
                    maximumValue: 100
                    value: page.percent
                    valueText: page.percent + "%"
                }

            }

            IconField {
                id: tagsField

                width: parent.width
                visible: page.tags.length > 0
                topPadding: Theme.paddingMedium
                description: qsTr("Tags")
                icon.source: Qt.resolvedUrl("../icons/icon-m-tag.svg")
                icon.color: Theme.primaryColor

                TagChips {
                    x: tagsField.grooveLeftMargin
                    width: parent.width - x - tagsField.grooveRightMargin
                    tags: page.tags
                }

            }

            SectionHeader {
                visible: page.startDate !== "" || page.dueDate !== "" || page.reminder >= 0
                text: qsTr("Schedule")
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

            SectionHeader {
                visible: page.location !== "" || page.url !== "" || page.description !== ""
                text: qsTr("Details")
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

            IconField {
                id: notesField

                width: parent.width
                visible: page.description !== ""
                description: qsTr("Notes")
                icon.source: "image://theme/icon-m-note"
                // A note sits at its natural height so the caption follows the
                // text, not a padded band. When that height is shorter than the
                // icon, the row is padded so the icon centres on the text
                // instead of climbing above the row; a long note keeps the icon
                // on its first line.
                lineCenterY: Math.min(notesLabel.height, Theme.itemSizeSmall) / 2
                topPadding: Math.max(0, (Theme.iconSizeMedium - notesLabel.height) / 2)

                Label {
                    id: notesLabel

                    x: notesField.grooveLeftMargin
                    width: parent.width - x - notesField.grooveRightMargin
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText
                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeMedium
                    linkColor: Theme.highlightColor
                    text: Askare.renderNotes(page.description)
                    onLinkActivated: Qt.openUrlExternally(link)
                }

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
