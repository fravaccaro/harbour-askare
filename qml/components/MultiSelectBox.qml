/*!
    \qmltype MultiSelectBox
    \brief An IconComboBox whose menu lets several options be on at once.

    The box never collapses to a single choice: its menu is a ContextMenu of
    Opal.MenuSwitch toggles, and \c currentIndex is held at -1. The description
    reads \c placeholder until one or more options are on, then names those.

    \c defaultIcon is the leading icon while nothing (or more than one option)
    is selected. Each entry of \c options may carry an optional \c icon; that
    icon is shown beside the switch in the menu, and replaces \c defaultIcon on
    the closed box when that option is the only one selected.

    \c options is a list of \c {{ value, text, icon? }}. \c selected is the list
    of chosen values, which the box both reads and writes; \l selectionChanged
    reports every change. The control carries no backend reference.

    \qml
    MultiSelectBox {
        title: "Status"
        defaultIcon: "image://theme/icon-m-diagnostic"
        options: [
            { "value": "a", "text": "A", "icon": "image://theme/icon-m-accept" },
            { "value": "b", "text": "B", "icon": "image://theme/icon-m-cancel" }
        ]
        onSelectionChanged: console.log(values)
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0
import Opal.MenuSwitch 1.0

IconComboBox {
    id: root

    property string title
    property string placeholder: qsTr("Any")
    property string defaultIcon
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

    function optionIcon(value) {
        for (var i = 0; i < root.options.length; i++) {
            if (root.options[i].value === value && root.options[i].icon)
                return root.options[i].icon
        }
        return ""
    }

    function activeIcon() {
        if (root.selected.length === 1) {
            var chosen = root.optionIcon(root.selected[0])
            if (chosen !== "")
                return chosen
        }
        return root.defaultIcon
    }

    width: parent.width
    label: root.title
    currentIndex: -1
    description: root.summary()
    icon.source: root.activeIcon()
    onCurrentIndexChanged: if (currentIndex >= 0) currentIndex = -1

    menu: ContextMenu {
        Repeater {
            model: root.options

            MenuSwitch {
                text: modelData.text
                automaticCheck: false
                checked: root.isSelected(modelData.value)
                // Switch, then the option icon when one is set, then the label.
                // The vendored switch centres its label; a multi-select list
                // reads better with the switch and text ranged left.
                horizontalAlignment: Text.AlignLeft
                leftPadding: switchItem.width + Theme.paddingLarge
                             + (modelData.icon ? Theme.iconSizeMedium + Theme.paddingMedium : 0)
                onClicked: root.toggle(modelData.value)

                HighlightImage {
                    visible: modelData.icon !== undefined && modelData.icon !== ""
                    x: parent.switchItem.width + Theme.paddingSmall
                    anchors.verticalCenter: parent.verticalCenter
                    sourceSize.width: Theme.iconSizeMedium
                    sourceSize.height: Theme.iconSizeMedium
                    highlighted: parent.highlighted
                    source: modelData.icon ? modelData.icon : ""
                }

            }

        }

    }

}
