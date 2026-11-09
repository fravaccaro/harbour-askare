/*!
    \qmltype MultiSelectBox
    \brief An IconComboBox whose menu lets several options be on at once.

    The box never collapses to a single choice: its menu is a ContextMenu of
    left-ranged rows with a Switch on the right, and \c currentIndex is held at
    -1. The description reads \c placeholder until one or more options are on,
    then names those.

    \c defaultIcon is the leading icon while nothing (or more than one option)
    is selected. Each entry of \c options may carry an optional \c icon; that
    icon is shown beside the label in the menu, and replaces \c defaultIcon on
    the closed box when that option is the only one selected. The labels hold
    one text column either way, so a menu of plain rows lines up with one whose
    rows are led by icons.

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
import "Selection.js" as Selection

IconComboBox {
    id: root

    property string title
    property string placeholder: qsTr("Any")
    property string defaultIcon
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

    function summary() {
        if (root.selected.length === 0)
            return root.placeholder;

        var names = [];
        for (var i = 0; i < root.options.length; i++) {
            if (root.isSelected(root.options[i].value))
                names.push(root.options[i].text);

        }
        return names.join(", ");
    }

    function optionIcon(value) {
        for (var i = 0; i < root.options.length; i++) {
            if (root.options[i].value === value && root.options[i].icon)
                return root.options[i].icon;

        }
        return "";
    }

    function activeIcon() {
        if (root.selected.length === 1) {
            var chosen = root.optionIcon(root.selected[0]);
            if (chosen !== "")
                return chosen;

        }
        return root.defaultIcon;
    }

    width: parent.width
    label: root.title
    currentIndex: -1
    description: root.summary()
    icon.source: root.activeIcon()
    onCurrentIndexChanged: {
        if (currentIndex >= 0) {
            currentIndex = -1;
        }
    }

    menu: ContextMenu {
        Repeater {
            model: root.options

            MenuItem {
                text: modelData.text
                horizontalAlignment: Text.AlignLeft
                // The text column is held whether or not the option carries an
                // icon, so a menu of plain rows reads down the same edge as one
                // whose rows are led by icons. Switch on the right.
                leftPadding: Theme.iconSizeMedium + Theme.paddingMedium
                rightPadding: Theme.itemSizeExtraSmall
                onClicked: root.toggle(modelData.value)

                HighlightImage {
                    visible: modelData.icon !== undefined && modelData.icon !== ""
                    anchors.verticalCenter: parent.verticalCenter
                    sourceSize.width: Theme.iconSizeMedium
                    sourceSize.height: Theme.iconSizeMedium
                    highlighted: parent.highlighted
                    source: modelData.icon ? modelData.icon : ""
                }

                Switch {
                    // Display only: ContextMenu fires MenuItem.clicked for the
                    // whole row, including taps on the switch.

                    width: Theme.itemSizeExtraSmall
                    height: Theme.itemSizeSmall
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    automaticCheck: false
                    checked: root.isSelected(modelData.value)
                }

            }

        }

    }

}
