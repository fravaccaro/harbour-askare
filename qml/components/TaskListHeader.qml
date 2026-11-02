/*!
    \qmltype TaskListHeader
    \brief The header of the task list: its name and the search field.

    Sits above the tasks as the list's own header. It keeps no model: the
    caller passes \c title and \c searchText and answers \c searchRequested.

    The field follows \c searchVisible and looks after its own focus. Showing
    it is a request to type in it; hiding it calls the search off, which empties
    the field and so reports an empty \c searchRequested. \c searchText is what
    the field starts from when the header is rebuilt with the search already
    open, as happens when the view's layout is reassembled.

    \qml
    TaskListHeader {
        width: parent.width
        title: Tasks.title
        searchVisible: page.searchVisible
        searchText: Tasks.search
        onSearchRequested: Tasks.search = text
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    id: root

    property string title
    property bool searchVisible: false
    property string searchText: ""

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
