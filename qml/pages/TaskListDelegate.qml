/*!
    \qmltype TaskListDelegate
    \brief A task row wired to the session, for the task list and the search page.

    TaskRow draws the row. This binds it to the task being shown and to the
    actions the two lists share, so neither page repeats that wiring.

    \qml
    SilicaListView {
        model: Tasks
        delegate: TaskListDelegate {}
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

TaskRow {
    width: ListView.view.width
    summary: model.summary
    dueDate: model.dueDate
    dueTime: model.dueTime
    done: model.done
    cancelled: model.group === "cancelled"
    showListColor: Tasks.filter !== "calendar"
    listColor: model.calendarColor
    childCount: model.childCount
    childDoneCount: model.childDoneCount
    subtreeSize: function() { return Askare.subtreeSize(model.href) }
    onClicked: app.pushTask(model.href)
    onToggled: Askare.toggleDone(model.href)
    onEditRequested: app.editTask(model.href)
    onDeleteConfirmed: Askare.deleteTask(model.href)
}
