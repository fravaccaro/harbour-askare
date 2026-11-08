/*!
    \qmltype MultiSelectBox
    \brief A ComboBox whose menu lets several options be on at once.

    The box never collapses to a single choice: its menu is a ContextMenu of
    Opal.MenuSwitch toggles, and \c currentIndex is held at -1. The description
    reads \c placeholder until one or more options are on, then names those.

    \c options is a list of \c {{ value, text }}. \c selected is the list of
    chosen values, which the box both reads and writes; \l selectionChanged
    reports every change. The control carries no backend reference.

    \qml
    MultiSelectBox {
        title: "Status"
        options: [{ "value": "a", "text": "A" }, { "value": "b", "text": "B" }]
        onSelectionChanged: console.log(values)
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0
import Opal.MenuSwitch 1.0

ComboBox {
    id: root

    property string title
    property string placeholder: qsTr("Any")
    property var options: []
    property var selected: []

    signal selectionChanged(var values)

    function isSelected(value) {
        return root.selected.indexOf(value) >= 0
    }

    function toggle(value) {
        var out = []
        for (var i = 0; i < root.selected.length; i++) {
            if (root.selected[i] !== value)
                out.push(root.selected[i])
        }
        if (!root.isSelected(value))
            out.push(value)
        root.selected = out
        root.selectionChanged(out)
    }

    function clear() {
        if (root.selected.length === 0)
            return
        root.selected = []
        root.selectionChanged([])
    }

    function summary() {
        if (root.selected.length === 0)
            return root.placeholder
        var names = []
        for (var i = 0; i < root.options.length; i++) {
            if (root.isSelected(root.options[i].value))
                names.push(root.options[i].text)
        }
        return names.join(", ")
    }

    width: parent.width
    label: root.title
    currentIndex: -1
    description: root.summary()
    onCurrentIndexChanged: if (currentIndex >= 0) currentIndex = -1

    menu: ContextMenu {
        Repeater {
            model: root.options

            MenuSwitch {
                text: modelData.text
                automaticCheck: false
                checked: root.isSelected(modelData.value)
                // The vendored switch centres its label; a multi-select list
                // reads better with the switch and text ranged left.
                horizontalAlignment: Text.AlignLeft
                leftPadding: switchItem.width + Theme.paddingLarge
                onClicked: root.toggle(modelData.value)
            }

        }

    }

}
