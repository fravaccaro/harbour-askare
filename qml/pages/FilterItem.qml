/*!
    \qmltype FilterItem
    \brief A list row showing a named view and how many tasks it holds.

    Used for the All, Current, Today and Completed entries of the lists page, for
    the tags, and for the saved smart lists. The row carries no model knowledge:
    the caller supplies the name, the count and whether the view is the selected
    one, and reacts to \c clicked.

    A \l deletable row offers Delete on press and hold, behind a remorse timer,
    and reports the answer as \l deleteConfirmed.

    \qml
    FilterItem {
        name: qsTr("Current")
        count: 4
        selected: true
        onClicked: Tasks.showCurrent()
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

ListItem {
    id: root

    property string name
    property int count: 0
    property bool selected: false
    property bool deletable: false

    signal deleteConfirmed()

    function confirmDelete() {
        root.remorseAction(qsTr("Deleting"), function() {
            root.deleteConfirmed();
        });
    }

    contentHeight: Theme.itemSizeSmall
    // A row nobody can delete keeps no menu, so press and hold does nothing
    // on the built-in views and the tags.
    menu: root.deletable ? deleteMenu : null

    Label {
        truncationMode: TruncationMode.Fade
        color: root.selected || root.highlighted ? Theme.highlightColor : Theme.primaryColor
        text: root.name

        anchors {
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
            right: countLabel.left
            rightMargin: Theme.paddingMedium
            verticalCenter: parent.verticalCenter
        }

    }

    Label {
        id: countLabel

        color: Theme.secondaryColor
        font.pixelSize: Theme.fontSizeSmall
        text: root.count

        anchors {
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
            verticalCenter: parent.verticalCenter
        }

    }

    Component {
        id: deleteMenu

        ContextMenu {
            MenuItem {
                text: qsTr("Delete")
                onClicked: root.confirmDelete()
            }

        }

    }

}
