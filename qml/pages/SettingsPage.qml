import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

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
                description: qsTr("Use this only for a server whose certificate the device does not trust.")
                checked: Askare.ignoreSslErrors
                onCheckedChanged: Askare.ignoreSslErrors = checked
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                visible: Askare.errorMessage !== ""
                color: Theme.errorColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: Askare.errorMessage
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
