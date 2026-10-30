/*!
    \qmltype TagEditor
    \brief Edits the tags of a task as removable chips.

    The component owns no model: the caller hands in \c tags as an array of
    strings and reads the same property back when saving. \c suggestions names
    the tags already used elsewhere; the ones the task does not carry yet are
    offered below the field and are added by a tap.

    A tag is kept once whatever its case, and blank input is ignored.

    \qml
    TagEditor {
        tags: ["Work"]
        suggestions: ["Work", "Home"]
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    id: root

    property var tags: []
    property var suggestions: []
    // The tags on offer: those used elsewhere that this task does not carry.
    readonly property var available: root.unusedOf(root.suggestions, root.tags)

    function has(name) {
        for (var i = 0; i < root.tags.length; ++i) {
            if (root.tags[i].toLowerCase() === name.toLowerCase())
                return true;

        }
        return false;
    }

    // An array is replaced rather than changed in place, so the bindings that
    // read it follow.
    function addTag(name) {
        var clean = name.replace(/^\s+|\s+$/g, "");
        if (clean === "" || root.has(clean))
            return ;

        root.tags = root.tags.concat([clean]);
    }

    function removeAt(index) {
        var next = root.tags.slice();
        next.splice(index, 1);
        root.tags = next;
    }

    function unusedOf(all, used) {
        var out = [];
        for (var i = 0; i < all.length; ++i) {
            if (!root.has(all[i]))
                out.push(all[i]);

        }
        return out;
    }

    spacing: Theme.paddingSmall

    Label {
        x: Theme.horizontalPageMargin
        color: Theme.secondaryColor
        font.pixelSize: Theme.fontSizeSmall
        text: qsTr("Tags")
    }

    Flow {
        x: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin
        spacing: Theme.paddingSmall

        Label {
            visible: root.tags.length === 0
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeSmall
            text: qsTr("None")
        }

        Repeater {
            model: root.tags

            Rectangle {
                width: chipRow.width + 2 * Theme.paddingMedium
                height: chipRow.height + Theme.paddingSmall
                radius: height / 2
                color: Theme.rgba(Theme.highlightColor, chipArea.pressed ? 0.4 : 0.2)

                Row {
                    id: chipRow

                    anchors.centerIn: parent
                    spacing: Theme.paddingSmall

                    Label {
                        color: Theme.highlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        text: modelData
                    }

                    Label {
                        color: Theme.highlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        text: "\u00d7"
                    }

                }

                MouseArea {
                    id: chipArea

                    anchors.fill: parent
                    onClicked: root.removeAt(index)
                }

            }

        }

    }

    TextField {
        id: entry

        width: parent.width
        label: qsTr("New tag")
        placeholderText: qsTr("New tag")
        leftItem: Icon {
            source: "image://theme/icon-m-favorite"
        }
        inputMethodHints: Qt.ImhNoAutoUppercase
        EnterKey.iconSource: "image://theme/icon-m-add"
        EnterKey.onClicked: {
            root.addTag(entry.text);
            entry.text = "";
        }
    }

    Label {
        x: Theme.horizontalPageMargin
        visible: root.available.length > 0
        color: Theme.secondaryColor
        font.pixelSize: Theme.fontSizeSmall
        text: qsTr("Tags already in use")
    }

    Flow {
        x: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin
        spacing: Theme.paddingSmall

        Repeater {
            model: root.available

            Rectangle {
                width: offerLabel.width + 2 * Theme.paddingMedium
                height: offerLabel.height + Theme.paddingSmall
                radius: height / 2
                border.width: 1
                border.color: Theme.rgba(Theme.highlightColor, 0.4)
                color: offerArea.pressed ? Theme.rgba(Theme.highlightColor, 0.2) : "transparent"

                Label {
                    id: offerLabel

                    anchors.centerIn: parent
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    text: modelData
                }

                MouseArea {
                    id: offerArea

                    anchors.fill: parent
                    onClicked: root.addTag(modelData)
                }

            }

        }

    }

}
