/*!
    \qmltype FilterItem
    \brief A list row showing a named view and how many tasks it holds.

    Used for the All, Current and Completed entries of the lists page. The row
    carries no model knowledge: the caller supplies the name, the count and
    whether the view is the selected one, and reacts to \c clicked.

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

    contentHeight: Theme.itemSizeSmall

    Label {
        anchors {
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
            right: countLabel.left
            rightMargin: Theme.paddingMedium
            verticalCenter: parent.verticalCenter
        }
        truncationMode: TruncationMode.Fade
        color: root.selected || root.highlighted ? Theme.highlightColor : Theme.primaryColor
        text: root.name
    }

    Label {
        id: countLabel

        anchors {
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
            verticalCenter: parent.verticalCenter
        }
        color: Theme.secondaryColor
        font.pixelSize: Theme.fontSizeSmall
        text: root.count
    }
}
