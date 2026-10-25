import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property string errorMessage

    function beginLogin() {
        page.errorMessage = ""
        Askare.startLogin(serverField.text)
    }

    allowedOrientations: Orientation.All

    Connections {
        target: Askare
        onLoginUrlReady: pageStack.push(Qt.resolvedUrl("WebLoginPage.qml"), {
                                            "loginUrl": url
                                        })
        onLoginFailed: page.errorMessage = error
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Askare")
            }

            Image {
                width: Theme.itemSizeExtraLarge
                height: width
                fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
                source: "../../appicon.png"
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Askare connects to a Nextcloud server that you already run.")
            }

            TextField {
                id: serverField

                width: parent.width
                text: Askare.serverUrl
                label: qsTr("Server address")
                placeholderText: qsTr("https://cloud.example.org")
                inputMethodHints: Qt.ImhUrlCharactersOnly | Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: page.beginLogin()
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
                color: Theme.errorColor
                font.pixelSize: Theme.fontSizeExtraSmall
                visible: page.errorMessage !== ""
                text: page.errorMessage
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: serverField.text.trim() !== ""
                text: qsTr("Sign in")
                onClicked: page.beginLogin()
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("The server opens inside the app. Sign in as you would in a browser. Askare then keeps the password the server creates.")
            }
        }

        VerticalScrollDecorator {
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: Askare.busy
    }
}
