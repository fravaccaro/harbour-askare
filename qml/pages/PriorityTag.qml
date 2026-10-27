/*!
    \qmltype PriorityTag
    \brief Shows a task priority band as a colored dot with its name.

    Bands follow Nextcloud and RFC 5545: 0 is no priority, 1-4 high, 5 medium,
    6-9 low. The tag hides itself when \c priority is 0.

    \c colorFor() and \c labelFor() expose the same mapping to callers that only
    need the value, such as a Slider's valueText.

    \qml
    PriorityTag { priority: 3 }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

Row {
    id: root

    property int priority: 0

    function colorFor(value) {
        if (value >= 1 && value <= 4)
            return "#d9534f";

        if (value === 5)
            return "#f0ad4e";

        if (value >= 6 && value <= 9)
            return "#5cb85c";

        return "transparent";
    }

    function labelFor(value) {
        if (value >= 1 && value <= 4)
            return qsTr("High");

        if (value === 5)
            return qsTr("Medium");

        if (value >= 6 && value <= 9)
            return qsTr("Low");

        return qsTr("No priority");
    }

    spacing: Theme.paddingSmall
    visible: root.priority > 0

    Rectangle {
        width: Theme.fontSizeSmall
        height: width
        radius: width / 2
        anchors.verticalCenter: parent.verticalCenter
        color: root.colorFor(root.priority)
    }

    Label {
        anchors.verticalCenter: parent.verticalCenter
        color: Theme.secondaryColor
        font.pixelSize: Theme.fontSizeSmall
        text: root.labelFor(root.priority)
    }

}
