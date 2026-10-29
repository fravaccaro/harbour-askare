/*!
    \qmltype StatusOptions
    \brief The iCalendar task states Askare can store, with their labels.

    A task's STATUS is held as its iCalendar token. This object is not visual:
    instantiate it wherever the list of states or a label for one is needed.

    \qml
    StatusOptions { id: states }
    // states.labelFor("IN-PROCESS") -> "In process"
    \endqml
*/
import QtQuick 2.0

QtObject {
    id: root

    readonly property var values: ["NEEDS-ACTION", "IN-PROCESS", "COMPLETED", "CANCELLED"]

    function indexOf(status) {
        var index = root.values.indexOf(status);
        return index < 0 ? 0 : index;
    }

    function labelFor(status) {
        if (status === "IN-PROCESS")
            return qsTr("In process");

        if (status === "COMPLETED")
            return qsTr("Completed");

        if (status === "CANCELLED")
            return qsTr("Cancelled");

        return qsTr("Needs action");
    }

}
