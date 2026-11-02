/*!
    \qmltype ConflictField
    \brief One field two versions of a task disagree on, with both readings.

    Shows the name of the field and, under it, what each side says, so the two
    can be read against one another. It holds no model and asks nothing of the
    session: the caller passes \c label and the two values already in words.

    An empty value reads as \c emptyText rather than as a blank line, so a
    field one side cleared still shows what it means.

    \qml
    ConflictField {
        width: parent.width
        label: qsTr("Due")
        mine: "2026-11-02 14:00"
        theirs: ""
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    id: root

    property string label
    property string mine
    property string theirs
    property string emptyText: qsTr("None")

    spacing: Theme.paddingSmall

    Label {
        x: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin
        truncationMode: TruncationMode.Fade
        color: Theme.secondaryHighlightColor
        font.pixelSize: Theme.fontSizeExtraSmall
        text: root.label
    }

    Label {
        x: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin
        wrapMode: Text.WordWrap
        color: Theme.primaryColor
        font.pixelSize: Theme.fontSizeSmall
        text: qsTr("On this phone: %1").arg(root.mine === "" ? root.emptyText : root.mine)
    }

    Label {
        x: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin
        wrapMode: Text.WordWrap
        color: Theme.primaryColor
        font.pixelSize: Theme.fontSizeSmall
        text: qsTr("On the server: %1").arg(root.theirs === "" ? root.emptyText : root.theirs)
    }

}
