import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    function selectAll() {
        Tasks.showAll();
        pageStack.navigateBack();
    }

    function selectCurrent() {
        Tasks.showCurrent();
        pageStack.navigateBack();
    }

    function selectImportant() {
        Tasks.showImportant();
        pageStack.navigateBack();
    }

    function selectCompleted() {
        Tasks.showCompleted();
        pageStack.navigateBack();
    }

    function selectCalendar(href, name) {
        Tasks.showCalendar(href, name);
        pageStack.navigateBack();
    }

    function selectTag(name) {
        Tasks.showTag(name);
        pageStack.navigateBack();
    }

    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView

        anchors.fill: parent
        model: Calendars

        ViewPlaceholder {
            enabled: listView.count === 0 && !Askare.busy
            text: qsTr("No task lists")
            hintText: qsTr("Pull down to refresh")
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

        }

        VerticalScrollDecorator {
        }

        header: Column {
            width: listView.width

            PageHeader {
                title: qsTr("Lists")
            }

            FilterItem {
                width: parent.width
                name: qsTr("Important")
                count: Askare.importantCount
                selected: Tasks.filter === "important"
                onClicked: page.selectImportant()
            }

            FilterItem {
                width: parent.width
                name: qsTr("All")
                count: Askare.openCount
                selected: Tasks.filter === "all"
                onClicked: page.selectAll()
            }

            FilterItem {
                width: parent.width
                name: qsTr("Current")
                count: Askare.currentCount
                selected: Tasks.filter === "current"
                onClicked: page.selectCurrent()
            }

            FilterItem {
                width: parent.width
                name: qsTr("Completed")
                count: Askare.completedCount
                selected: Tasks.filter === "completed"
                onClicked: page.selectCompleted()
            }

            SectionHeader {
                visible: Askare.tags.length > 0
                text: qsTr("Tags")
            }

            Repeater {
                model: Askare.tags

                FilterItem {
                    width: listView.width
                    name: modelData.name
                    count: modelData.count
                    selected: Tasks.filter === "tag" && Tasks.tag === modelData.name
                    onClicked: page.selectTag(modelData.name)
                }

            }

            SectionHeader {
                text: qsTr("Task lists")
            }

        }

        delegate: ListItem {
            id: delegate

            readonly property bool selected: Tasks.filter === "calendar" && Tasks.calendarHref === href

            contentHeight: Theme.itemSizeSmall
            onClicked: page.selectCalendar(href, displayName)

            Rectangle {
                id: colorDot

                width: Theme.fontSizeMedium
                height: width
                radius: width / 2
                color: model.color !== "" ? model.color : Theme.secondaryColor

                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }

            }

            Label {
                truncationMode: TruncationMode.Fade
                color: delegate.selected || delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                text: displayName

                anchors {
                    left: colorDot.right
                    leftMargin: Theme.paddingMedium
                    right: countLabel.left
                    rightMargin: Theme.paddingMedium
                    verticalCenter: parent.verticalCenter
                }

            }

            Label {
                id: countLabel

                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: openCount

                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }

            }

        }

    }

    BusyIndicator {
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: Askare.busy && listView.count === 0
    }

}
