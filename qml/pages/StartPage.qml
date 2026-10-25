import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    readonly property bool stalled: !Askare.ready && Askare.secretsError !== ""

    allowedOrientations: Orientation.All

    BusyIndicator {
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: !page.stalled
        visible: running
    }

    Column {
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width
        spacing: Theme.paddingLarge
        visible: page.stalled

        Label {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            color: Theme.highlightColor
            text: qsTr("The password could not be read from the secure storage of Sailfish OS.")
        }

        Label {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.secondaryColor
            text: Askare.secretsError
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Try again")
            onClicked: Askare.reloadSecrets()
        }
    }
}
