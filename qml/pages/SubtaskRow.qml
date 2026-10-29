/*!
    \qmltype SubtaskRow
    \brief A compact row for one subtask on its parent's detail page.

    Shows a done toggle, the title, and a due hint, and leaves navigation to
    the caller. Press and hold offers Edit, the done toggle, and Delete, the
    last behind a remorse timer counting \l subtreeCount tasks. It carries no
    backend reference, so it can be previewed on its own and reused wherever a
    task needs a one-line summary.

    A \l done row is struck through. A \l cancelled one is struck through and
    muted, and drops the toggle from both the row and the menu, since it is not
    work waiting to be finished.

    \qml
    SubtaskRow {
        summary: "Buy milk"
        dueText: "20 Nov 09:30"
        done: false
        cancelled: false
        subtreeCount: Askare.subtreeSize(href)
        onToggled: Askare.toggleDone(href)
        onEditRequested: app.editTask(href)
        onDeleteConfirmed: Askare.deleteTask(href)
        onClicked: app.pushTask(href)
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

ListItem {
    id: root

    property string summary
    property string dueText
    property bool done: false
    // A cancelled subtask reads as struck through and muted, and carries no
    // done toggle: it is not work waiting to be finished.
    property bool cancelled: false
    // How many tasks a delete would take with it, this one included.
    property int subtreeCount: 1

    signal toggled()
    signal editRequested()
    signal deleteConfirmed()

    contentHeight: Theme.itemSizeSmall

    TextSwitch {
        id: doneSwitch

        width: root.cancelled ? 0 : Theme.itemSizeSmall
        visible: !root.cancelled
        checked: root.done
        automaticCheck: false
        onClicked: root.toggled()

        anchors {
            left: parent.left
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
            truncationMode: TruncationMode.Fade
            color: root.highlighted ? Theme.highlightColor : (root.cancelled ? Theme.secondaryColor : Theme.primaryColor)
            font.pixelSize: Theme.fontSizeSmall
            font.strikeout: root.done || root.cancelled
            text: root.summary
        }

        Label {
            width: parent.width
            visible: root.dueText !== ""
            truncationMode: TruncationMode.Fade
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
            text: root.dueText
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
            onClicked: {
                var label = root.subtreeCount > 1 ? qsTr("Deleting %n tasks", "", root.subtreeCount) : qsTr("Deleting");
                root.remorseAction(label, function() {
                    root.deleteConfirmed();
                });
            }
        }

    }

}
