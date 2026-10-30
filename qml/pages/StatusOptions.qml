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

    // "" is a task with no STATUS at all, the state a new task starts in on
    // Nextcloud; it leads the list so clearing the state reads as the default.
    readonly property var values: ["", "NEEDS-ACTION", "IN-PROCESS", "COMPLETED", "CANCELLED"]

    function indexOf(status) {
        var index = root.values.indexOf(status);
        return index < 0 ? 0 : index;
    }

    function labelFor(status) {
        if (status === "NEEDS-ACTION")
            return qsTr("Needs action");

        if (status === "IN-PROCESS")
            return qsTr("In process");

        if (status === "COMPLETED")
            return qsTr("Completed");

        if (status === "CANCELLED")
            return qsTr("Cancelled");

        return qsTr("No status");
    }

    // A theme icon that stands for each state, so a row reads at a glance.
    function iconFor(status) {
        if (status === "IN-PROCESS")
            return "image://theme/icon-m-refresh";

        if (status === "COMPLETED")
            return "image://theme/icon-m-accept";

        if (status === "CANCELLED")
            return "image://theme/icon-m-cancel";

        if (status === "NEEDS-ACTION")
            return "image://theme/icon-m-diagnostic";

        return "image://theme/icon-m-question";
    }

}
