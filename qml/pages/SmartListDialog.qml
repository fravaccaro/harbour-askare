/*!
    \qmltype SmartListDialog
    \brief Asks for the name a saved filter set is filed under.

    The dialog holds nothing but the name, and cannot be accepted while that is
    blank. The caller reads \l name once accepted and saves the filters itself,
    so the dialog carries no backend reference.

    \qml
    var dialog = pageStack.push(Qt.resolvedUrl("SmartListDialog.qml"))
    dialog.accepted.connect(function() {
        Askare.saveSmartList(dialog.name, Tasks.filters)
    })
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: dialog

    // Trimmed, so a name of spaces is neither accepted nor saved.
    readonly property string name: nameField.text.trim()

    allowedOrientations: Orientation.All
    canAccept: dialog.name.length > 0

    Column {
        width: parent.width

        DialogHeader {
            acceptText: qsTr("Save")
            title: qsTr("New smart list")
        }

        TextField {
            id: nameField

            width: parent.width
            label: qsTr("Name")
            placeholderText: qsTr("Name")
            EnterKey.iconSource: "image://theme/icon-m-enter-close"
            EnterKey.onClicked: focus = false
            Component.onCompleted: forceActiveFocus()

            leftItem: Icon {
                source: "image://theme/icon-m-levels"
            }

        }

        Label {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            wrapMode: Text.WordWrap
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
            text: qsTr("Smart lists save your active filters to automatically group matching tasks.")
        }

    }

}
