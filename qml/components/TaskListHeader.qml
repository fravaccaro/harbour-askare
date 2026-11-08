/*!
    \qmltype TaskListHeader
    \brief The header of the task list: its name, and a row when a sync conflict is waiting.

    Sits above the tasks as the list's own header. It keeps no model: the caller
    passes \c title and \c conflictCount and answers \c conflictClicked.

    The conflict row only appears while \c conflictCount is above zero.

    \qml
    TaskListHeader {
        width: parent.width
        title: Tasks.title
        conflictCount: Askare.conflictCount
        onConflictClicked: pageStack.push(Qt.resolvedUrl("ConflictPage.qml"))
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    id: root

    property string title
    property int conflictCount: 0

    signal conflictClicked()

    PageHeader {
        title: root.title
    }

    // The conflict page is never pushed on its own: a page arriving over the
    // user's work during a sync would be an ambush. The row waits here
    // instead, for as long as anything is unsettled.
    BackgroundItem {
        id: conflictRow

        width: parent.width
        visible: root.conflictCount > 0
        onClicked: root.conflictClicked()

        Label {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            anchors.verticalCenter: parent.verticalCenter
            truncationMode: TruncationMode.Fade
            color: conflictRow.highlighted ? Theme.highlightColor : Theme.errorColor
            font.pixelSize: Theme.fontSizeSmall
            text: qsTr("%n task(s) changed on the server as well", "", root.conflictCount)
        }

    }

}
