import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    function selectAll() {
        Tasks.showAll()
        pageStack.navigateBack()
    }

    function selectCurrent() {
        Tasks.showCurrent()
        pageStack.navigateBack()
    }

    function selectCalendar(href, name) {
        Tasks.showCalendar(href, name)
        pageStack.navigateBack()
    }

    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView

        anchors.fill: parent
        model: Calendars

        header: Column {
            width: listView.width

            PageHeader {
                title: qsTr("Lists")
            }

            ListItem {
                id: allItem

                width: parent.width
                contentHeight: Theme.itemSizeMedium
                onClicked: page.selectAll()

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    truncationMode: TruncationMode.Fade
                    color: Tasks.filter === "all" || allItem.highlighted
                           ? Theme.highlightColor : Theme.primaryColor
                    text: qsTr("All")
                }

                Label {
                    anchors {
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    text: Askare.openCount
                }
            }

            ListItem {
                id: currentItem

                width: parent.width
                contentHeight: Theme.itemSizeMedium
                onClicked: page.selectCurrent()

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    truncationMode: TruncationMode.Fade
                    color: Tasks.filter === "current" || currentItem.highlighted
                           ? Theme.highlightColor : Theme.primaryColor
                    text: qsTr("Current")
                }

                Label {
                    anchors {
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    text: Askare.currentCount
                }
            }
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
            onClicked: page.selectCalendar(href, displayName)

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                truncationMode: TruncationMode.Fade
                    color: (Tasks.filter === "calendar" && Tasks.calendarHref === href)
                           || delegate.highlighted
                           ? Theme.highlightColor : Theme.primaryColor
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
