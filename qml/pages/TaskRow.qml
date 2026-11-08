/*!
    \qmltype TaskRow
    \brief One task in a list: its list colour, done toggle, title and due line.

    Press and hold offers Edit, the done toggle, and Delete, the last behind a
    remorse timer. \c subtreeSize is called only when Delete is chosen, so a
    long list does not count descendants for every row. The row carries no
    backend reference.

    A \l done row is struck through. A \l cancelled one is struck through and
    muted, and drops the toggle from both the row and the menu.

    \qml
    TaskRow {
        summary: "Buy milk"
        dueDate: "2026-11-08"
        dueTime: "09:30"
        done: false
        showListColor: true
        listColor: "#e08a1e"
        childCount: 2
        childDoneCount: 1
        subtreeSize: function() { return 1 }
        onToggled: Askare.toggleDone(href)
        onClicked: app.pushTask(href)
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

ListItem {
    id: root

    property string summary
    property string dueDate
    property string dueTime
    property bool done: false
    property bool cancelled: false
    property bool showListColor: false
    property string listColor
    property int childCount: 0
    property int childDoneCount: 0
    // Read when Delete is chosen. Defaults to this task alone.
    property var subtreeSize: function() { return 1 }

    signal toggled()
    signal editRequested()
    signal deleteConfirmed()

    function dueLine() {
        if (root.dueDate === "")
            return ""
        if (root.dueTime === "")
            return root.dueDate
        return root.dueDate + " " + root.dueTime
    }

    function confirmDelete() {
        var total = root.subtreeSize()
        var label = total > 1 ? qsTr("Deleting %n tasks", "", total) : qsTr("Deleting")
        root.remorseAction(label, function() {
            root.deleteConfirmed()
        })
    }

    contentHeight: Theme.itemSizeMedium

    Rectangle {
        id: listColorMark

        width: Math.round(6 * Theme.pixelRatio)
        height: parent.height - 2 * Theme.paddingMedium
        radius: width / 2
        // Every row in a mixed view keeps the bar so the rows stay aligned,
        // even where the list was left without a colour.
        visible: root.showListColor
        color: root.listColor !== "" ? root.listColor : Theme.secondaryColor

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }

    }

    TextSwitch {
        id: doneSwitch

        width: root.cancelled ? 0 : Theme.itemSizeSmall
        visible: !root.cancelled
        checked: root.done
        automaticCheck: false
        onClicked: root.toggled()

        anchors {
            left: parent.left
            leftMargin: listColorMark.visible ? listColorMark.width + Theme.paddingSmall : 0
            verticalCenter: parent.verticalCenter
        }

    }

    Column {
        anchors {
            left: doneSwitch.right
            // With no toggle to indent it, a cancelled row takes the page
            // margin instead of sitting against the edge.
            leftMargin: root.cancelled ? Theme.horizontalPageMargin : 0
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
            verticalCenter: parent.verticalCenter
        }

        Label {
            width: parent.width
            // The row is a fixed height, so the title holds to one line: a
            // summary carrying a line break would otherwise spill over.
            maximumLineCount: 1
            truncationMode: TruncationMode.Fade
            color: root.highlighted ? Theme.highlightColor : (root.cancelled ? Theme.secondaryColor : Theme.primaryColor)
            font.strikeout: root.done || root.cancelled
            text: root.summary
        }

        Label {
            width: parent.width
            visible: root.dueLine() !== ""
            truncationMode: TruncationMode.Fade
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
            text: root.dueLine()
        }

        Label {
            width: parent.width
            visible: root.childCount > 0
            truncationMode: TruncationMode.Fade
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
            text: qsTr("Subtasks %1/%2").arg(root.childDoneCount).arg(root.childCount)
        }

    }

    menu: ContextMenu {
        MenuItem {
            text: qsTr("Edit")
            onClicked: root.editRequested()
        }

        MenuItem {
            visible: !root.cancelled
            text: root.done ? qsTr("Mark to-do") : qsTr("Mark done")
            onClicked: root.toggled()
        }

        MenuItem {
            text: qsTr("Delete")
            onClicked: root.confirmDelete()
        }

    }

}
