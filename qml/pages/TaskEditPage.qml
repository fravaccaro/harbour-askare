import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property string href
    property string calendarHref
    property string summary
    property string description
    property string dueDate
    property int priority: 0
    property bool done: false

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

    function save() {
        Askare.saveTask(page.href, page.calendarHref, summaryField.text,
                        notesField.text, page.dueDate, page.priority, page.done)
        pageStack.pop()
    }

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("Save")
                onClicked: page.save()
            }
        }

        Column {
            id: column

            width: page.width

            PageHeader {
                title: page.href === "" ? qsTr("New task") : qsTr("Task")
            }

            TextField {
                id: summaryField

                width: parent.width
                text: page.summary
                label: qsTr("Title")
                placeholderText: qsTr("Title")
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Completed")
                checked: page.done
                onCheckedChanged: page.done = checked
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

            ComboBox {
                width: parent.width
                label: qsTr("Priority")
                currentIndex: page.priority === 1 ? 1 : page.priority === 9 ? 3 : page.priority > 0 ? 2 : 0
                onCurrentIndexChanged: {
                    var values = [0, 1, 5, 9]
                    page.priority = values[currentIndex]
                }

                menu: ContextMenu {
                    MenuItem { text: qsTr("None") }
                    MenuItem { text: qsTr("High") }
                    MenuItem { text: qsTr("Normal") }
                    MenuItem { text: qsTr("Low") }
                }
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
