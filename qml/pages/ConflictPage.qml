import "../components"
import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    // The task being settled, or null once the last one has been answered.
    // Reading conflictCount is what brings the next one back after every
    // answer, since the session reports both through the same signal.
    property var conflict: Askare.conflictCount > 0 ? Askare.conflicts()[0] : null

    // Priority, status and reminder reach the page as they are stored, so each
    // is put through the object that already holds the words for it.
    function wordsFor(field, value) {
        if (field === "priority")
            return priorityBands.labelFor(value);

        if (field === "status")
            return statusOptions.labelFor(value);

        if (field === "reminder")
            return reminderOptions.labelFor(value);

        if (field === "percent")
            return value + "%";

        return value;
    }

    allowedOrientations: Orientation.All

    // Instantiated only for their words, so each mapping stays in one place.
    PriorityTag {
        id: priorityBands

        visible: false
    }

    StatusOptions {
        id: statusOptions
    }

    ReminderOptions {
        id: reminderOptions
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            visible: page.conflict !== null

            MenuItem {
                text: qsTr("Keep the server's version")
                onClicked: Askare.resolveKeepTheirs(page.conflict.href)
            }

            MenuItem {
                text: qsTr("Keep my version")
                onClicked: Askare.resolveKeepMine(page.conflict.href)
            }

        }

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Which version?")
                description: page.conflict ? page.conflict.summary : ""
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                visible: page.conflict !== null
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("This task was changed here and on the server. Pull down to keep one version or the other.")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                visible: Askare.conflictCount > 1
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("%n more task(s) wait after this one.", "", Askare.conflictCount - 1)
            }

            Repeater {
                model: page.conflict ? page.conflict.differences : []

                ConflictField {
                    width: column.width
                    label: modelData.label
                    mine: page.wordsFor(modelData.field, modelData.mine)
                    theirs: page.wordsFor(modelData.field, modelData.theirs)
                }

            }

        }

        // The questions are not kept anywhere: an app closed before they were
        // answered has none to show until a sync meets the same tasks again.
        ViewPlaceholder {
            enabled: page.conflict === null
            text: qsTr("Nothing to settle")
            hintText: qsTr("A task changed on both sides is found again by the next sync")
        }

        VerticalScrollDecorator {
        }

    }

}
