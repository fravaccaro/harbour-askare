import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.WebView 1.0

Page {
    id: page

    property string loginUrl
    property string errorMessage

    allowedOrientations: Orientation.All
    Component.onDestruction: {
        if (!Askare.configured)
            Askare.cancelLogin()
    }

    Connections {
        target: Askare
        onLoginSucceeded: pageStack.replaceAbove(null, Qt.resolvedUrl("CalendarsPage.qml"))
        onLoginFailed: page.errorMessage = error
    }

    WebView {
        id: webview

        url: page.loginUrl
        active: true

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: footer.top
        }
    }

    Rectangle {
        anchors.fill: webview
        color: Theme.overlayBackgroundColor
        visible: Askare.configured
    }

    Column {
        id: footer

        spacing: Theme.paddingSmall

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        Label {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            wrapMode: Text.WordWrap
            font.pixelSize: Theme.fontSizeExtraSmall
            color: page.errorMessage !== "" ? Theme.errorColor : Theme.secondaryColor
            text: page.errorMessage !== ""
                  ? page.errorMessage
                  : qsTr("Sign in as you would in a browser. Askare waits for the server to grant access.")
        }

        Item {
            width: parent.width
            height: Theme.paddingMedium
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: Askare.configured
    }
}
