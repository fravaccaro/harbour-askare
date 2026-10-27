import Opal.LinkHandler 1.0 as L
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
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Translations")
            }

            DetailItem {
                label: "Italiano"
                value: "fravaccaro"
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                text: qsTr("Request a new language or contribute to existing languages on the Transifex project page.")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Transifex"
                onClicked: L.LinkHandler.openOrCopyUrl("https://app.transifex.com/fravaccaro/askare", text)
            }

        }

        VerticalScrollDecorator {
        }

    }

}
