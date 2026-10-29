/*!
    \qmltype TagChips
    \brief Shows the tags of a task as a row of rounded chips.

    Read-only counterpart of TagEditor. The component carries no model
    knowledge: the caller hands in \c tags as an array of strings. An empty
    array draws the \c placeholderText instead, so a task without tags still
    takes a line and reads like the other fields.

    \qml
    TagChips { tags: ["Work", "Urgent"] }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

Flow {
    id: root

    property var tags: []
    property string placeholderText: qsTr("None")

    spacing: Theme.paddingSmall

    Label {
        visible: root.tags.length === 0
        color: Theme.secondaryColor
        font.pixelSize: Theme.fontSizeSmall
        text: root.placeholderText
    }

    Repeater {
        model: root.tags

        Rectangle {
            width: chipLabel.width + 2 * Theme.paddingMedium
            height: chipLabel.height + Theme.paddingSmall
            radius: height / 2
            color: Theme.rgba(Theme.highlightColor, 0.2)

            Label {
                id: chipLabel

                anchors.centerIn: parent
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: modelData
            }

        }

    }

}
