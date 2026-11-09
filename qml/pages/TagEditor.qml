import "../components"
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

    IconField {
        id: chipsField

        width: parent.width
        topPadding: Theme.paddingMedium
        description: qsTr("Tags")
        icon.source: Qt.resolvedUrl("../icons/icon-m-tag.svg")
        icon.color: Theme.primaryColor

        Flow {
            x: chipsField.grooveLeftMargin
            width: parent.width - x - chipsField.grooveRightMargin
            spacing: Theme.paddingSmall

            Label {
                visible: root.tags.length === 0
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("None")
            }

            Repeater {
                model: root.tags

                TagChip {
                    text: modelData
                    suffix: "\u00d7"
                    onClicked: root.removeAt(index)
                }

            }

        }

    }


    Label {
        x: chipsField.contentLeft
        visible: root.available.length > 0
        color: Theme.secondaryColor
        font.pixelSize: Theme.fontSizeSmall
    }

    Flow {
        x: chipsField.contentLeft
        width: parent.width - x - Theme.horizontalPageMargin
        spacing: Theme.paddingSmall

        Repeater {
            model: root.available

            TagChip {
                text: modelData
                filled: false
                onClicked: root.addTag(modelData)
            }

        }

    }


    TextField {
        id: entry

        width: parent.width
        textLeftMargin: chipsField.contentLeft
        label: qsTr("New tag")
        placeholderText: qsTr("Add a tag...")
        inputMethodHints: Qt.ImhNoAutoUppercase
        EnterKey.iconSource: "image://theme/icon-m-add"
        EnterKey.onClicked: {
            root.addTag(entry.text);
            entry.text = "";
        }
    }

}
