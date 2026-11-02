/*!
    \qmltype TaskListHeader
    \brief The header of the task list: its name and the search field.

    Sits above the tasks as the list's own header. It keeps no model: the caller
    passes \c title, \c conflictCount and \c searchText and answers the signals.

    The conflict row only appears while \c conflictCount is above zero. The
    field follows \c searchVisible and looks after its own focus. Showing
    it is a request to type in it; hiding it calls the search off, which empties
    the field and so reports an empty \c searchRequested. \c searchText is what
    the field starts from when the header is rebuilt with the search already
    open, as happens when the view's layout is reassembled.

    \qml
    TaskListHeader {
        width: parent.width
        title: Tasks.title
        conflictCount: Askare.conflictCount
        searchVisible: page.searchVisible
        searchText: Tasks.search
        onConflictClicked: pageStack.push(Qt.resolvedUrl("ConflictPage.qml"))
        onSearchRequested: Tasks.search = text
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    id: root

    property string title
    property int conflictCount: 0
    property bool searchVisible: false
    property string searchText: ""

    signal conflictClicked()
    signal searchRequested(string text)

    function applySearchVisible() {
        if (root.searchVisible) {
            if (searchField.text !== root.searchText)
                searchField.text = root.searchText;

            searchField.forceActiveFocus();
            return ;
        }
        searchField.text = "";
        searchField.focus = false;
    }

    onSearchVisibleChanged: root.applySearchVisible()
    Component.onCompleted: root.applySearchVisible()

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

    SearchField {
        id: searchField

        width: parent.width
        visible: root.searchVisible
        placeholderText: qsTr("Search tasks")
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
        EnterKey.iconSource: "image://theme/icon-m-enter-close"
        EnterKey.onClicked: focus = false
        onTextChanged: root.searchRequested(text)
    }

}
