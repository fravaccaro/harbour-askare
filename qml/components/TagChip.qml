/*!
    \qmltype TagChip
    \brief One tag as a rounded chip.

    A \l filled chip reads as on - a tag a task carries, or one a filter keeps -
    and an outlined chip as off, a tag merely on offer. \l suffix draws a glyph
    after the name, such as the cross of a chip a tap takes away.

    The chip is a MouseArea, so it reports a tap as \c clicked and shows the
    press. A chip that is only there to be read sets \c enabled to false, which
    lets the press through to whatever lies under it.

    \qml
    TagChip {
        text: "Work"
        filled: false
        onClicked: console.log("Work")
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

MouseArea {
    id: root

    property string text
    property bool filled: true
    property string suffix

    width: chipRow.width + 2 * Theme.paddingMedium
    height: chipRow.height + Theme.paddingSmall

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        border.width: root.filled ? 0 : 1
        border.color: Theme.rgba(Theme.highlightColor, 0.4)
        color: root.filled ? Theme.rgba(Theme.highlightColor, root.pressed ? 0.4 : 0.2) : root.pressed ? Theme.rgba(Theme.highlightColor, 0.2) : "transparent"
    }

    Row {
        id: chipRow

        anchors.centerIn: parent
        spacing: Theme.paddingSmall

        Label {
            color: root.filled ? Theme.highlightColor : Theme.secondaryColor
            font.pixelSize: Theme.fontSizeSmall
            text: root.text
        }

        Label {
            visible: root.suffix !== ""
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeSmall
            text: root.suffix
        }

    }

}
