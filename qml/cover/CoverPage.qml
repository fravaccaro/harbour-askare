import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    clip: true

    HighlightImage {
        source: Qt.resolvedUrl("cover-check.png")
        color: Theme.primaryColor
        // The tick is wider than it is tall, so its width is what decides how
        // much of the cover it takes.
        sourceSize.width: parent.width * 0.85

        // The tip of the long arm and the point of the turn cross the edges, so
        // the tick sits into the corner rather than floating inside it.
        anchors {
            right: parent.right
            rightMargin: -width * 0.08
            bottom: parent.bottom
            bottomMargin: -height * 0.04
        }

        // The same alpha reads much heavier as a dark glyph on a light ambience.
        opacity: Theme.colorScheme === Theme.LightOnDark ? 0.13 : 0.08
    }

    Column {
        spacing: Theme.paddingSmall

        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            margins: Theme.paddingMedium
        }

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            color: Theme.secondaryHighlightColor
            font.pixelSize: Theme.fontSizeSmall
            text: qsTr("Askare")
        }

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            visible: Askare.configured && Askare.openCount > 0
            color: Theme.highlightColor
            text: visible ? Askare.openCount : ""

            font {
                pixelSize: Theme.fontSizeHuge
                family: Theme.fontFamilyHeading
            }
        }

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
            text: !Askare.configured
                  ? qsTr("Not signed in")
                  : Askare.openCount === 0
                    ? qsTr("No open tasks")
                    : Askare.nextDueSummary !== ""
                      ? Askare.nextDueSummary
                      : qsTr("open", "goes under the number of open tasks")
        }
    }

    CoverActionList {
        enabled: Askare.configured
        CoverAction {
            iconSource: "image://theme/icon-cover-sync"
            onTriggered: Askare.sync()
        }
    }
}
