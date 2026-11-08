import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    function clearQuery() {
        Tasks.search = ""
    }

    allowedOrientations: Orientation.All
    // A page pushed on top deactivates this one too. Back is what says the
    // search itself is leaving, so the list underneath is whole again before
    // it shows. A task opened from a result leaves the query in place.
    onStatusChanged: {
        if (status === PageStatus.Active)
            searchField.forceActiveFocus()

        if (status === PageStatus.Deactivating && page._navigation === PageNavigation.Back)
            page.clearQuery()

    }
    Component.onDestruction: page.clearQuery()

    Column {
        id: heading

        width: parent.width

        PageHeader {
            title: qsTr("Search")
        }

        SearchField {
            id: searchField

            width: parent.width
            placeholderText: qsTr("Search tasks")
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
            EnterKey.iconSource: "image://theme/icon-m-enter-close"
            EnterKey.onClicked: focus = false
            onTextChanged: Tasks.search = text
        }

    }

    SilicaListView {
        id: listView

        model: Tasks
        // Nothing here is picked out. A current row would take the focus the
        // field just gained, and every keystroke rebuilds the model.
        currentIndex: -1
        section.property: "group"

        anchors {
            top: heading.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        ViewPlaceholder {
            enabled: listView.count === 0 && !Askare.busy
            text: Tasks.search !== "" ? qsTr("No matches") : qsTr("No tasks")
            hintText: Tasks.search !== "" ? qsTr("Try another search") : ""
        }

        VerticalScrollDecorator {
        }

        section.delegate: SectionHeader {
            text: section === "cancelled" ? qsTr("Cancelled") : (section === "completed" ? qsTr("Completed") : qsTr("To do"))
        }

        delegate: TaskListDelegate {
        }

    }

}
