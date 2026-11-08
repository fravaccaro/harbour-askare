import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"

Page {
    id: page

    function openSearch() {
        pageStack.push(Qt.resolvedUrl("SearchPage.qml"))
    }

    allowedOrientations: Orientation.All
    onStatusChanged: {
        if (status === PageStatus.Active && pageStack.nextPage() === null)
            pageStack.pushAttached(Qt.resolvedUrl("ListsPage.qml"));

    }

    SilicaListView {
        id: listView

        model: Tasks
        // No row here is picked out, so none is left looking pressed after a
        // sync rebuilds the model.
        currentIndex: -1
        section.property: "group"

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        ViewPlaceholder {
            enabled: listView.count === 0 && !Askare.busy
            text: qsTr("No tasks")
            hintText: qsTr("Pull down to add a task")
        }

        PullDownMenu {
            busy: Askare.busy

            MenuItem {
                text: qsTr("Refresh")
                onClicked: Askare.sync()
            }

            MenuItem {
                text: qsTr("Search")
                onClicked: page.openSearch()
            }

            MenuItem {
                enabled: Askare.configured && Calendars.count > 0
                text: qsTr("New task")
                onClicked: pageStack.push(Qt.resolvedUrl("TaskEditPage.qml"), {
                    "calendarHref": Tasks.calendarHref
                })
            }

        }

        VerticalScrollDecorator {
        }

        section.delegate: SectionHeader {
            text: section === "cancelled" ? qsTr("Cancelled") : (section === "completed" ? qsTr("Completed") : qsTr("To do"))
        }

        header: TaskListHeader {
            width: listView.width
            title: Tasks.title
            conflictCount: Askare.conflictCount
            onConflictClicked: pageStack.push(Qt.resolvedUrl("ConflictPage.qml"))
        }

        delegate: TaskListDelegate {
        }

    }

}
