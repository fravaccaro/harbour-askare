import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All
    onStatusChanged: {
        if (status === PageStatus.Active && pageStack.nextPage() === null)
            pageStack.pushAttached(Qt.resolvedUrl("ListsPage.qml"))
    }

    Label {
        id: errorLabel

        x: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin
        wrapMode: Text.WordWrap
        visible: Askare.errorMessage !== ""
        color: Theme.errorColor
        font.pixelSize: Theme.fontSizeExtraSmall
        text: Askare.errorMessage
        anchors.top: parent.top
        anchors.topMargin: Theme.paddingMedium
    }

    SilicaListView {
        id: listView

        anchors {
            top: errorLabel.visible ? errorLabel.bottom : parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        model: Tasks
        section.property: "done"
        section.delegate: SectionHeader {
            text: section === "true" || section === true ? qsTr("Completed") : qsTr("Open")
        }

        header: PageHeader {
            title: Tasks.title
        }

        ViewPlaceholder {
            enabled: listView.count === 0 && !Askare.busy
            text: qsTr("No tasks")
            hintText: qsTr("Pull down to add a task")
        }

        PullDownMenu {
            busy: Askare.busy
            MenuItem {
                text: qsTr("Sync")
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

        delegate: ListItem {
            id: delegate

            contentHeight: Theme.itemSizeMedium
            onClicked: app.pushTask(href)
            menu: ContextMenu {
                MenuItem {
                    text: done ? qsTr("Mark open") : qsTr("Mark done")
                    onClicked: Askare.toggleDone(href)
                }
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: {
                        var total = Askare.subtreeSize(href)
                        var label = total > 1 ? qsTr("Deleting %n tasks", "", total)
                                              : qsTr("Deleting")
                        remorseAction(label, function() { Askare.deleteTask(href) })
                    }
                }
            }

            TextSwitch {
                id: doneSwitch

                width: Theme.itemSizeSmall
                checked: done
                automaticCheck: false
                onClicked: Askare.toggleDone(href)

                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
            }

            Column {
                anchors {
                    left: doneSwitch.right
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }

                Label {
                    width: parent.width
                    truncationMode: TruncationMode.Fade
                    color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                    font.strikeout: done
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
        }

        VerticalScrollDecorator {
        }
    }
}
