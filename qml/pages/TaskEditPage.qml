import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: page

    property string href
    property string calendarHref
    property string summary
    property string description
    property string dueDate
    property int priority: 0
    property int percent: 0
    property bool listIndexReady: false

    function dueAsDate() {
        if (page.dueDate === "")
            return new Date()
        var parts = page.dueDate.split("-")
        return new Date(Number(parts[0]), Number(parts[1]) - 1, Number(parts[2]))
    }

    function applyDue(dialog) {
        var month = dialog.month < 10 ? "0" + dialog.month : dialog.month
        var day = dialog.day < 10 ? "0" + dialog.day : dialog.day
        page.dueDate = dialog.year + "-" + month + "-" + day
    }

    function pickDue() {
        var dialog = pageStack.push("Sailfish.Silica.DatePickerDialog", {
                                        "date": page.dueAsDate()
                                    })
        dialog.accepted.connect(function() { page.applyDue(dialog) })
    }

    function chosenCalendar() {
        if (page.href !== "" && page.calendarHref !== "")
            return page.calendarHref
        var index = listBox.currentIndex < 0 ? 0 : listBox.currentIndex
        return Calendars.hrefAt(index)
    }

    function applyListIndex() {
        if (page.listIndexReady || Calendars.count === 0)
            return
        listBox.currentIndex = Calendars.indexOfHref(page.calendarHref)
        page.listIndexReady = true
    }

    allowedOrientations: Orientation.All
    canAccept: page.chosenCalendar() !== "" && summaryField.text.length > 0
    onAccepted: Askare.saveTask(page.href, page.chosenCalendar(), summaryField.text,
                                notesField.text, page.dueDate, page.priority,
                                page.percent)

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: page.width

            DialogHeader {
                acceptText: page.href === "" ? qsTr("Add task") : qsTr("Save")
                title: page.href === "" ? qsTr("New task") : qsTr("Task")
            }

            ComboBox {
                id: listBox

                width: parent.width
                visible: page.href === ""
                label: qsTr("List")
                Component.onCompleted: page.applyListIndex()

                menu: ContextMenu {
                    Repeater {
                        model: Calendars
                        MenuItem {
                            text: displayName
                        }
                    }
                }

                Connections {
                    target: Calendars
                    onCountChanged: page.applyListIndex()
                }
            }

            TextField {
                id: summaryField

                width: parent.width
                text: page.summary
                label: qsTr("Title")
                placeholderText: qsTr("Title")
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: notesField.focus = true
            }

            Slider {
                width: parent.width
                label: qsTr("Completion")
                minimumValue: 0
                maximumValue: 100
                stepSize: 5
                valueText: value + "%"
                Component.onCompleted: value = page.percent
                onValueChanged: page.percent = value
            }

            Slider {
                width: parent.width
                label: qsTr("Priority")
                minimumValue: 0
                maximumValue: 9
                stepSize: 1
                valueText: value === 0 ? qsTr("None") : value
                Component.onCompleted: value = page.priority
                onValueChanged: page.priority = value
            }

            ValueButton {
                width: parent.width
                label: qsTr("Due")
                value: page.dueDate === "" ? qsTr("None") : page.dueDate
                onClicked: page.pickDue()
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: page.dueDate !== ""
                text: qsTr("Clear due date")
                onClicked: page.dueDate = ""
            }

            TextArea {
                id: notesField

                width: parent.width
                text: page.description
                label: qsTr("Notes")
                placeholderText: qsTr("Notes")
            }
        }

        VerticalScrollDecorator {
        }
    }
}
