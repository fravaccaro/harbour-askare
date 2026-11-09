import "../components"
/*!
    \qmltype SearchFilters
    \brief The search page's filter form, shown in a drawer.

    Each control writes the whole filter set to \c Tasks.filters as soon as it
    changes, so the results update while the drawer is open. \l reset() clears
    every control and the model together. Icons and the completion field follow
    the same IconComboBox / IconField / ValueButton pattern as the edit page.
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

SilicaFlickable {
    id: form

    // While a reset runs, each control's change is ignored, so the model is
    // written once at the end instead of once per control.
    property bool resetting: false
    property string startDate
    property string startMode: "on"
    property string dueDate
    property string dueMode: "on"
    property string completionMode: "atLeast"
    readonly property var dateModes: ["on", "before", "after"]
    readonly property var completionModes: ["atLeast", "atMost"]
    readonly property var reminderModes: ["any", "yes", "no"]
    // The empty first span is Any, which leaves the filter off. The labels run
    // beside the spans, so the menu and what it writes stay in step.
    readonly property var dueWithinModes: ["", "today", "week", "month"]
    readonly property var dueWithinLabels: [qsTr("Any"), qsTr("Today"), qsTr("This week"), qsTr("This month")]
    readonly property var priorityOptions: [{
        "value": "none",
        "text": priorityHelper.labelFor(0),
        "icon": "image://theme/icon-m-levels"
    }, {
        "value": "high",
        "text": priorityHelper.labelFor(1),
        "icon": priorityHelper.iconFor(1)
    }, {
        "value": "medium",
        "text": priorityHelper.labelFor(5),
        "icon": priorityHelper.iconFor(5)
    }, {
        "value": "low",
        "text": priorityHelper.labelFor(6),
        "icon": priorityHelper.iconFor(6)
    }]
    property var statusOptions: statusOptionsOf(statusHelper.values)
    property var listOptions: listOptionsOf(Calendars.count)
    property var tagOptions: tagOptionsOf(Askare.tags)

    function statusOptionsOf(values) {
        var out = [];
        for (var i = 0; i < values.length; i++) {
            out.push({
                "value": values[i],
                "text": statusHelper.labelFor(values[i]),
                "icon": statusHelper.iconFor(values[i])
            });
        }
        return out;
    }

    function listOptionsOf(count) {
        var out = [];
        for (var i = 0; i < count; i++) {
            var href = Calendars.hrefAt(i);
            out.push({
                "value": href,
                "text": Calendars.nameOfHref(href)
            });
        }
        return out;
    }

    function tagOptionsOf(tags) {
        var out = [];
        for (var i = 0; i < tags.length; i++) out.push({
            "value": tags[i].name,
            "text": tags[i].name
        })
        return out;
    }

    function pad(value) {
        return value < 10 ? "0" + value : "" + value;
    }

    function asDate(iso) {
        if (iso === "")
            return new Date();

        var parts = iso.split("-");
        return new Date(Number(parts[0]), Number(parts[1]) - 1, Number(parts[2]));
    }

    function pickDate(iso, apply) {
        var dialog = pageStack.push("Sailfish.Silica.DatePickerDialog", {
            "date": form.asDate(iso)
        });
        dialog.accepted.connect(function() {
            apply(dialog.year + "-" + form.pad(dialog.month) + "-" + form.pad(dialog.day));
        });
    }

    function apply() {
        Tasks.filters = {
            "status": statusBox.selected,
            "lists": listBox.selected,
            "priority": priorityBox.selected,
            "tags": tagChips.selected,
            "completion": completionSlider.value,
            "completionMode": form.completionMode,
            "dueWithin": form.dueWithinModes[dueWithinBox.currentIndex],
            "startDate": form.startDate,
            "startMode": form.startMode,
            "dueDate": form.dueDate,
            "dueMode": form.dueMode,
            "reminder": form.reminderModes[reminderBox.currentIndex]
        };
    }

    function reset() {
        form.resetting = true;
        statusBox.clear();
        listBox.clear();
        priorityBox.clear();
        tagChips.clear();
        completionSlider.value = 0;
        form.completionMode = "atLeast";
        dueWithinBox.currentIndex = 0;
        form.startDate = "";
        form.startMode = "on";
        form.dueDate = "";
        form.dueMode = "on";
        reminderBox.currentIndex = 0;
        form.resetting = false;
        Tasks.filters = {
        };
    }

    contentHeight: column.height + Theme.paddingLarge

    StatusOptions {
        id: statusHelper
    }

    PriorityTag {
        id: priorityHelper

        visible: false
    }

    Column {
        id: column

        width: form.width

        PageHeader {
            title: qsTr("Filters")
        }

        MultiSelectBox {
            id: listBox

            title: qsTr("List")
            defaultIcon: "image://theme/icon-m-events"
            options: form.listOptions
            onSelectionChanged: {
                if (!form.resetting)
                    form.apply();

            }
        }

        MultiSelectBox {
            id: statusBox

            title: qsTr("Status")
            defaultIcon: "image://theme/icon-m-diagnostic"
            options: form.statusOptions
            onSelectionChanged: {
                if (!form.resetting)
                    form.apply();

            }
        }

        MultiSelectBox {
            id: priorityBox

            title: qsTr("Priority")
            defaultIcon: "image://theme/icon-m-levels"
            options: form.priorityOptions
            onSelectionChanged: {
                if (!form.resetting)
                    form.apply();

            }
        }

        IconField {
            id: completionField

            width: parent.width
            description: qsTr("Completion")
            icon.source: "image://theme/icon-m-acknowledge"
            lineCenterY: completionSlider.height - Theme.itemSizeSmall / 2
            lineBottomY: lineCenterY

            Slider {
                id: completionSlider

                width: parent.width
                leftMargin: completionField.grooveLeftMargin
                rightMargin: completionField.grooveRightMargin
                minimumValue: 0
                maximumValue: 100
                stepSize: 5
                valueText: value === 0 ? qsTr("Any") : value + "%"
                onValueChanged: {
                    if (!form.resetting)
                        form.apply();

                }
            }

        }

        ComboBox {
            width: parent.width
            visible: completionSlider.value > 0
            label: qsTr("Completion is")
            currentIndex: form.completionModes.indexOf(form.completionMode)
            onCurrentIndexChanged: {
                if (form.resetting || currentIndex < 0)
                    return ;

                form.completionMode = form.completionModes[currentIndex];
                form.apply();
            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("At least")
                }

                MenuItem {
                    text: qsTr("At most")
                }

            }

        }

        // The tags on offer are the ones already in use, so they read as the
        // chips the task editor offers rather than as a menu of their own.
        IconField {
            id: tagField

            width: parent.width
            topPadding: Theme.paddingMedium
            visible: form.tagOptions.length > 0
            description: qsTr("Tags")
            icon.source: Qt.resolvedUrl("../icons/icon-m-tag.svg")
            icon.color: Theme.primaryColor

            MultiSelectChips {
                id: tagChips

                x: tagField.grooveLeftMargin
                width: parent.width - x - tagField.grooveRightMargin
                options: form.tagOptions
                onSelectionChanged: {
                    if (!form.resetting)
                        form.apply();

                }
            }

        }

        SectionHeader {
            text: qsTr("Schedule")
        }

        // The three spans are mutually exclusive, so they read as one choice
        // rather than as three toggles.
        IconComboBox {
            id: dueWithinBox

            width: parent.width
            label: qsTr("Due within")
            icon.source: "image://theme/icon-m-day-view"
            currentIndex: 0
            onCurrentIndexChanged: {
                if (!form.resetting)
                    form.apply();

            }

            // A ContextMenu centres its rows. These read down the same left
            // edge as the option menus above them instead.
            menu: ContextMenu {
                Repeater {
                    model: form.dueWithinLabels

                    MenuItem {
                        text: modelData
                        horizontalAlignment: Text.AlignLeft
                        leftPadding: Theme.iconSizeMedium + Theme.paddingMedium
                    }

                }

            }

        }

        ValueButton {
            width: parent.width
            label: qsTr("Start date")
            value: form.startDate === "" ? qsTr("Any") : form.startDate
            labelMargin: leftMargin + Theme.iconSizeMedium + Theme.paddingMedium
            minimumContentHeight: Theme.itemSizeMedium + Theme.paddingSmall
            onClicked: form.pickDate(form.startDate, function(iso) {
                form.startDate = iso;
                form.apply();
            })

            Icon {
                source: "image://theme/icon-m-date"

                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }

            }

        }

        ComboBox {
            width: parent.width
            visible: form.startDate !== ""
            label: qsTr("Start is")
            currentIndex: form.dateModes.indexOf(form.startMode)
            onCurrentIndexChanged: {
                if (form.resetting || currentIndex < 0)
                    return ;

                form.startMode = form.dateModes[currentIndex];
                form.apply();
            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("On")
                }

                MenuItem {
                    text: qsTr("Before")
                }

                MenuItem {
                    text: qsTr("After")
                }

            }

        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: form.startDate !== ""
            text: qsTr("Clear start date")
            onClicked: {
                form.startDate = "";
                form.apply();
            }
        }

        ValueButton {
            width: parent.width
            label: qsTr("Due date")
            value: form.dueDate === "" ? qsTr("Any") : form.dueDate
            labelMargin: leftMargin + Theme.iconSizeMedium + Theme.paddingMedium
            minimumContentHeight: Theme.itemSizeMedium + Theme.paddingSmall
            onClicked: form.pickDate(form.dueDate, function(iso) {
                form.dueDate = iso;
                form.apply();
            })

            Icon {
                source: "image://theme/icon-m-date"

                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }

            }

        }

        ComboBox {
            width: parent.width
            visible: form.dueDate !== ""
            label: qsTr("Due is")
            currentIndex: form.dateModes.indexOf(form.dueMode)
            onCurrentIndexChanged: {
                if (form.resetting || currentIndex < 0)
                    return ;

                form.dueMode = form.dateModes[currentIndex];
                form.apply();
            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("On")
                }

                MenuItem {
                    text: qsTr("Before")
                }

                MenuItem {
                    text: qsTr("After")
                }

            }

        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: form.dueDate !== ""
            text: qsTr("Clear due date")
            onClicked: {
                form.dueDate = "";
                form.apply();
            }
        }

        IconComboBox {
            id: reminderBox

            width: parent.width
            label: qsTr("Reminder set")
            icon.source: "image://theme/icon-m-alarm"
            currentIndex: 0
            onCurrentIndexChanged: {
                if (!form.resetting)
                    form.apply();

            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Any")
                }

                MenuItem {
                    text: qsTr("Yes")
                }

                MenuItem {
                    text: qsTr("No")
                }

            }

        }

        Item {
            width: parent.width
            height: Theme.paddingLarge
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            enabled: Tasks.filtersActive
            text: qsTr("Clear filters")
            onClicked: form.reset()
        }

    }

    VerticalScrollDecorator {
    }

}
