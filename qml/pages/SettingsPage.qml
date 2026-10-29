import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    readonly property var intervals: [0, 5, 15, 30, 60]

    function intervalIndex(minutes) {
        var index = page.intervals.indexOf(minutes);
        return index < 0 ? 0 : index;
    }

    function intervalLabel(minutes) {
        return minutes === 0 ? qsTr("Off") : qsTr("Every %n minute(s)", "", minutes);
    }

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: page.width

            PageHeader {
                title: qsTr("Settings")
            }

            SectionHeader {
                text: qsTr("Lists")
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Show completed tasks")
                description: qsTr("Completed tasks will appear under the Completed section.")
                checked: Askare.showCompletedTasks
                onCheckedChanged: Askare.showCompletedTasks = checked
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Show cancelled tasks")
                description: qsTr("Cancelled tasks will appear under the Cancelled section, where they cannot be changed.")
                checked: Askare.showCancelledTasks
                onCheckedChanged: Askare.showCancelledTasks = checked
            }

            SectionHeader {
                text: qsTr("Synchronisation")
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Refresh when the app starts")
                checked: Askare.syncOnStart
                onCheckedChanged: Askare.syncOnStart = checked
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Refresh after saving a task")
                description: qsTr("Fetches server changes once the task has been uploaded.")
                checked: Askare.syncAfterPush
                onCheckedChanged: Askare.syncAfterPush = checked
            }

            ComboBox {
                // Guards the stored value against the index changes that happen
                // while the menu is still being built.
                property bool ready: false

                width: parent.width
                label: qsTr("Automatic refresh")
                description: qsTr("Only while Askare is running.")
                Component.onCompleted: {
                    currentIndex = page.intervalIndex(Askare.syncInterval);
                    ready = true;
                }
                onCurrentIndexChanged: {
                    if (ready && currentIndex >= 0)
                        Askare.syncInterval = page.intervals[currentIndex];

                }

                menu: ContextMenu {
                    Repeater {
                        model: page.intervals

                        MenuItem {
                            text: page.intervalLabel(modelData)
                        }

                    }

                }

            }

            SectionHeader {
                text: qsTr("Connection")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                text: Askare.serverUrl
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Ignore certificate errors")
                description: qsTr("Only for a server with a self-signed certificate on your own network.")
                checked: Askare.ignoreSslErrors
                onCheckedChanged: Askare.ignoreSslErrors = checked
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Sign out")
                onClicked: remorse.execute(qsTr("Signing out"), Askare.logout)
            }

        }

        VerticalScrollDecorator {
        }

    }

    RemorsePopup {
        id: remorse
    }

}
