import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
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
