import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView

        anchors.fill: parent
        model: Calendars

        header: PageHeader {
            title: qsTr("Askare")
        }

        ViewPlaceholder {
            enabled: listView.count === 0 && !Askare.busy
            text: qsTr("No task lists")
            hintText: qsTr("Pull down to sync with the server")
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
                text: qsTr("Sync")
                onClicked: Askare.sync()
            }
        }

        delegate: ListItem {
            id: delegate

            contentHeight: Theme.itemSizeMedium
            onClicked: pageStack.push(Qt.resolvedUrl("TasksPage.qml"), {
                                          "calendarHref": href,
                                          "calendarName": displayName
                                      })

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                truncationMode: TruncationMode.Fade
                color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                text: displayName
            }

            Label {
                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: openCount
            }
        }

        VerticalScrollDecorator {
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: Askare.busy && listView.count === 0
    }
}
