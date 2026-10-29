import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All
    onStatusChanged: {
        if (status === PageStatus.Active && pageStack.nextPage() === null)
            pageStack.pushAttached(Qt.resolvedUrl("ListsPage.qml"));

    }

    SilicaListView {
        id: listView

        model: Tasks
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
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }

            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }

            MenuItem {
                text: qsTr("Refresh")
                onClicked: Askare.sync()
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

        header: PageHeader {
            title: Tasks.title
        }

        delegate: ListItem {
            id: delegate

            // A cancelled task reads as struck through and muted rather than
            // greyed out, so it can still be opened, edited and deleted. It
            // carries no done toggle: it is not a task waiting to be finished.
            readonly property bool cancelled: group === "cancelled"

            contentHeight: Theme.itemSizeMedium
            onClicked: app.pushTask(href)

            Rectangle {
                id: listColor

                width: Math.round(6 * Theme.pixelRatio)
                height: parent.height - 2 * Theme.paddingMedium
                radius: width / 2
                // Every row in a mixed view keeps the bar so the rows stay
                // aligned, even where the list was left without a colour.
                visible: Tasks.filter !== "calendar"
                color: calendarColor !== "" ? calendarColor : Theme.secondaryColor

                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }

            }

            TextSwitch {
                id: doneSwitch

                width: delegate.cancelled ? 0 : Theme.itemSizeSmall
                visible: !delegate.cancelled
                checked: done
                automaticCheck: false
                onClicked: Askare.toggleDone(href)

                anchors {
                    left: parent.left
                    leftMargin: listColor.visible ? listColor.width + Theme.paddingSmall : 0
                    verticalCenter: parent.verticalCenter
                }

            }

            Column {
                anchors {
                    left: doneSwitch.right
                    // With no toggle to indent it, a cancelled row takes the
                    // page margin instead of sitting against the edge.
                    leftMargin: delegate.cancelled ? Theme.horizontalPageMargin : 0
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }

                Label {
                    width: parent.width
                    truncationMode: TruncationMode.Fade
                    color: delegate.highlighted ? Theme.highlightColor : (delegate.cancelled ? Theme.secondaryColor : Theme.primaryColor)
                    font.strikeout: done || delegate.cancelled
                    text: summary
                }

                Label {
                    width: parent.width
                    visible: dueDate !== ""
                    truncationMode: TruncationMode.Fade
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                    text: dueTime === "" ? dueDate : dueDate + " " + dueTime
                }

                Label {
                    width: parent.width
                    visible: childCount > 0
                    truncationMode: TruncationMode.Fade
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                    text: qsTr("Subtasks %1/%2").arg(childDoneCount).arg(childCount)
                }

            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Edit")
                    onClicked: app.editTask(href)
                }

                MenuItem {
                    visible: !delegate.cancelled
                    text: done ? qsTr("Mark to-do") : qsTr("Mark done")
                    onClicked: Askare.toggleDone(href)
                }

                MenuItem {
                    text: qsTr("Delete")
                    onClicked: {
                        var total = Askare.subtreeSize(href);
                        var label = total > 1 ? qsTr("Deleting %n tasks", "", total) : qsTr("Deleting");
                        remorseAction(label, function() {
                            Askare.deleteTask(href);
                        });
                    }
                }

            }

        }

    }

}
