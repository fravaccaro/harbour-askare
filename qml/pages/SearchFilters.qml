/*!
    \qmltype SearchFilters
    \brief The search page's filter form, shown in a drawer.

    Each control writes the whole filter set to \c Tasks.filters as soon as it
    changes, so the results update while the drawer is open. \l reset() clears
    every control and the model together.
*/
import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"

SilicaFlickable {
    id: form

    // While a reset runs, each control's change is ignored, so the model is
    // written once at the end instead of once per control.
    property bool resetting: false
    property string startDate
    property string startMode: "on"
    property string dueDate
    property string dueMode: "on"

    readonly property var dateModes: ["on", "before", "after"]
    readonly property var reminderModes: ["any", "yes", "no"]

    readonly property var priorityOptions: [
        { "value": "none", "text": priorityHelper.labelFor(0) },
        { "value": "high", "text": priorityHelper.labelFor(1) },
        { "value": "medium", "text": priorityHelper.labelFor(5) },
        { "value": "low", "text": priorityHelper.labelFor(6) }
    ]
    property var statusOptions: statusOptionsOf(statusHelper.values)
    property var listOptions: listOptionsOf(Calendars.count)
    property var tagOptions: tagOptionsOf(Askare.tags)

    function statusOptionsOf(values) {
        var out = []
        for (var i = 0; i < values.length; i++)
            out.push({ "value": values[i], "text": statusHelper.labelFor(values[i]) })
        return out
    }

    function listOptionsOf(count) {
        var out = []
        for (var i = 0; i < count; i++) {
            var href = Calendars.hrefAt(i)
            out.push({ "value": href, "text": Calendars.nameOfHref(href) })
        }
        return out
    }

    function tagOptionsOf(tags) {
        var out = []
        for (var i = 0; i < tags.length; i++)
            out.push({ "value": tags[i].name, "text": tags[i].name })
        return out
    }

    function pad(value) {
        return value < 10 ? "0" + value : "" + value
    }

    function asDate(iso) {
        if (iso === "")
            return new Date()
        var parts = iso.split("-")
        return new Date(Number(parts[0]), Number(parts[1]) - 1, Number(parts[2]))
    }

    function pickDate(iso, apply) {
        var dialog = pageStack.push("Sailfish.Silica.DatePickerDialog", {
            "date": form.asDate(iso)
        })
        dialog.accepted.connect(function() {
            apply(dialog.year + "-" + form.pad(dialog.month) + "-" + form.pad(dialog.day))
        })
    }

    function apply() {
        Tasks.filters = {
            "status": statusBox.selected,
            "lists": listBox.selected,
            "priority": priorityBox.selected,
            "tags": tagBox.selected,
            "completion": completionSlider.value,
            "dueToday": dueTodaySwitch.checked,
            "startDate": form.startDate,
            "startMode": form.startMode,
            "dueDate": form.dueDate,
            "dueMode": form.dueMode,
            "reminder": form.reminderModes[reminderBox.currentIndex]
        }
    }

    function reset() {
        form.resetting = true
        statusBox.clear()
        listBox.clear()
        priorityBox.clear()
        tagBox.clear()
        completionSlider.value = 0
        dueTodaySwitch.checked = false
        form.startDate = ""
        form.startMode = "on"
        form.dueDate = ""
        form.dueMode = "on"
        reminderBox.currentIndex = 0
        form.resetting = false
        Tasks.filters = {}
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

        SectionHeader {
            text: qsTr("Filters")
        }

        MultiSelectBox {
            id: statusBox

            title: qsTr("Status")
            options: form.statusOptions
            onSelectionChanged: if (!form.resetting) form.apply()
        }

        MultiSelectBox {
            id: listBox

            title: qsTr("List")
            options: form.listOptions
            onSelectionChanged: if (!form.resetting) form.apply()
        }

        MultiSelectBox {
            id: priorityBox

            title: qsTr("Priority")
            options: form.priorityOptions
            onSelectionChanged: if (!form.resetting) form.apply()
        }

        MultiSelectBox {
            id: tagBox

            title: qsTr("Tags")
            visible: form.tagOptions.length > 0
            options: form.tagOptions
            onSelectionChanged: if (!form.resetting) form.apply()
        }

        Slider {
            id: completionSlider

            width: parent.width
            minimumValue: 0
            maximumValue: 100
            stepSize: 5
            label: qsTr("Completion")
            valueText: value === 0 ? qsTr("Any") : value + "%"
            onValueChanged: if (!form.resetting) form.apply()
        }

        TextSwitch {
            id: dueTodaySwitch

            text: qsTr("Tasks due today")
            onCheckedChanged: if (!form.resetting) form.apply()
        }

        SectionHeader {
            text: qsTr("Schedule")
        }

        ValueButton {
            width: parent.width
            label: qsTr("Start date")
            value: form.startDate === "" ? qsTr("Any") : form.startDate
            onClicked: form.pickDate(form.startDate, function(iso) {
                form.startDate = iso
                form.apply()
            })
        }

        ComboBox {
            width: parent.width
            visible: form.startDate !== ""
            label: qsTr("Start is")
            currentIndex: form.dateModes.indexOf(form.startMode)
            onCurrentIndexChanged: {
                if (form.resetting || currentIndex < 0)
                    return
                form.startMode = form.dateModes[currentIndex]
                form.apply()
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
                form.startDate = ""
                form.apply()
            }
        }

        ValueButton {
            width: parent.width
            label: qsTr("Due date")
            value: form.dueDate === "" ? qsTr("Any") : form.dueDate
            onClicked: form.pickDate(form.dueDate, function(iso) {
                form.dueDate = iso
                form.apply()
            })
        }

        ComboBox {
            width: parent.width
            visible: form.dueDate !== ""
            label: qsTr("Due is")
            currentIndex: form.dateModes.indexOf(form.dueMode)
            onCurrentIndexChanged: {
                if (form.resetting || currentIndex < 0)
                    return
                form.dueMode = form.dateModes[currentIndex]
                form.apply()
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
                form.dueDate = ""
                form.apply()
            }
        }

        SectionHeader {
            text: qsTr("Reminder")
        }

        ComboBox {
            id: reminderBox

            width: parent.width
            label: qsTr("Reminder set")
            currentIndex: 0
            onCurrentIndexChanged: if (!form.resetting) form.apply()

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
