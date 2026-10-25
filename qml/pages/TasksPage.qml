import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property string calendarHref
    property string calendarName

    allowedOrientations: Orientation.All
    onStatusChanged: {
        if (status === PageStatus.Activating)
            Tasks.calendarHref = page.calendarHref
    }

    SilicaListView {
        id: listView

        anchors.fill: parent
        model: Tasks
        section.property: "done"
        section.delegate: SectionHeader {
            text: section === "true" || section === true ? qsTr("Completed") : qsTr("Open")
        }

        header: PageHeader {
            title: page.calendarName
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
                text: qsTr("New task")
                onClicked: pageStack.push(Qt.resolvedUrl("TaskEditPage.qml"), {
                                              "calendarHref": page.calendarHref
                                          })
            }
        }

        delegate: ListItem {
            id: delegate

            contentHeight: Theme.itemSizeMedium
            onClicked: pageStack.push(Qt.resolvedUrl("TaskEditPage.qml"), {
                                          "href": href,
                                          "calendarHref": calendarHref,
                                          "summary": summary,
                                          "description": description,
                                          "dueDate": dueDate,
                                          "priority": priority,
                                          "done": done
                                      })
            menu: ContextMenu {
                MenuItem {
                    text: done ? qsTr("Mark open") : qsTr("Mark done")
                    onClicked: Askare.toggleDone(href)
                }
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: remorseAction(qsTr("Deleting"), function() {
                        Askare.deleteTask(href)
                    })
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
                    text: dueDate
                }
            }
        }

        VerticalScrollDecorator {
        }
    }
}
