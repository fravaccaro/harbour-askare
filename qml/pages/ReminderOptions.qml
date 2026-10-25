/*!
    \qmltype ReminderOptions
    \brief The reminder offsets Askare can store, with their labels.

    A reminder is held as the number of minutes before the due time, where 0
    means "at the due time" and -1 means the task has no reminder. This object
    is not visual: instantiate it wherever the list or a label is needed.

    \qml
    ReminderOptions { id: reminders }
    // reminders.labelFor(60) -> "1 hour before"
    \endqml
*/
import QtQuick 2.0

QtObject {
    id: root

    readonly property var values: [-1, 0, 5, 15, 30, 60, 120, 1440]

    function indexOf(minutes) {
        var index = root.values.indexOf(minutes)
        return index < 0 ? 0 : index
    }

    function labelFor(minutes) {
        if (minutes < 0)
            return qsTr("No reminder")
        if (minutes === 0)
            return qsTr("At the due time")
        if (minutes % 1440 === 0)
            return qsTr("%n day(s) before", "", minutes / 1440)
        if (minutes % 60 === 0)
            return qsTr("%n hour(s) before", "", minutes / 60)
        return qsTr("%n minute(s) before", "", minutes)
    }
}
