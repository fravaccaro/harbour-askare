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
        id: heading

        spacing: Theme.paddingSmall

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: Theme.paddingLarge
        }

        Label {
            width: parent.width
            truncationMode: TruncationMode.Fade
            color: Theme.secondaryHighlightColor
            font.pixelSize: Theme.fontSizeMedium
            text: Askare.configured ? Tasks.title : qsTr("Askare")
        }
    }

    ListView {
        id: taskList

        clip: true
        interactive: false
        spacing: Theme.paddingSmall
        model: Askare.configured ? Tasks : null

        anchors {
            top: heading.bottom
            topMargin: Theme.paddingLarge
            left: parent.left
            right: parent.right
            leftMargin: Theme.paddingLarge
            rightMargin: Theme.paddingLarge
            // Leave room for the cover actions so rows do not run behind them.
            bottom: parent.bottom
            bottomMargin: Theme.itemSizeSmall
        }

        delegate: Label {
            width: taskList.width
            truncationMode: TruncationMode.Fade
            font.pixelSize: Theme.fontSizeSmall
            font.strikeout: model.done
            color: model.done ? Theme.secondaryColor : Theme.primaryColor
            text: model.summary
        }
    }

    OpacityRampEffect {
        // The list runs under the bottom actions, so it fades out rather than
        // ending on a hard line of half-cut text. Direction 2 is top-to-bottom;
        // Silica exposes it only as a raw int.
        sourceItem: taskList
        direction: 2
        offset: 0.65
        slope: 2.0
        enabled: taskList.contentHeight > taskList.height
    }

    Label {
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            margins: Theme.paddingMedium
        }
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        visible: !Askare.configured || taskList.count === 0
        color: Theme.secondaryColor
        font.pixelSize: Theme.fontSizeExtraSmall
        text: !Askare.configured ? qsTr("Not signed in") : qsTr("No tasks")
    }

    CoverActionList {
        enabled: Askare.configured
        CoverAction {
            iconSource: "image://theme/icon-cover-new"
            // The cover is loaded outside the window's own file, so the window
            // is reached by the name Silica gives it.
            onTriggered: __silica_applicationwindow_instance.openNewTask()
        }
        CoverAction {
            iconSource: "image://theme/icon-cover-sync"
            onTriggered: Askare.sync()
        }
    }
}
