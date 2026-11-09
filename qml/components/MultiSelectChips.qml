/*!
    \qmltype MultiSelectChips
    \brief A flow of chips, several of which may be on at once.

    The chip counterpart of MultiSelectBox, for options that read better side by
    side than behind a menu. A tap puts an option in the selection or takes it
    out: a selected option is a filled chip, an unselected one an outlined chip.

    \c options is a list of \c {{ value, text }}. \c selected is the list of
    chosen values, which the flow both reads and writes; \l selectionChanged
    reports every change. The control carries no backend reference.

    \qml
    MultiSelectChips {
        options: [{ "value": "work", "text": "Work" }]
        onSelectionChanged: console.log(values)
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0
import "Selection.js" as Selection

Flow {
    id: root

    property var options: []
    property var selected: []

    signal selectionChanged(var values)

    function isSelected(value) {
        return Selection.has(root.selected, value);
    }

    function toggle(value) {
        root.selected = Selection.toggled(root.selected, value);
        root.selectionChanged(root.selected);
    }

    function clear() {
        if (root.selected.length === 0)
            return ;

        root.selected = [];
        root.selectionChanged([]);
    }

    spacing: Theme.paddingSmall

    Repeater {
        model: root.options

        TagChip {
            text: modelData.text
            filled: root.isSelected(modelData.value)
            onClicked: root.toggle(modelData.value)
        }

    }

}
