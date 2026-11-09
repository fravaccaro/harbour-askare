/*!
    \qmltype SmartListDialog
    \brief Asks for the name a saved search is filed under.

    The dialog holds nothing but the name, which starts as \l suggestion and
    cannot be accepted while blank. The caller reads \l name once accepted and
    saves the search itself, so the dialog carries no backend reference.

    \qml
    var dialog = pageStack.push(Qt.resolvedUrl("SmartListDialog.qml"),
                                { "suggestion": Tasks.search })
    dialog.accepted.connect(function() {
        Askare.saveSmartList(dialog.name, Tasks.searchSet())
    })
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: dialog

    // What the name starts as, which the caller sets to the words searched for.
    property string suggestion
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
            text: dialog.suggestion
            label: qsTr("Name")
            placeholderText: qsTr("Name")
            EnterKey.iconSource: "image://theme/icon-m-enter-close"
            EnterKey.onClicked: focus = false
            Component.onCompleted: forceActiveFocus()

            leftItem: Icon {
                source: "image://theme/icon-m-edit"
            }

        }

        Label {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            wrapMode: Text.WordWrap
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
            text: qsTr("Smart lists save what you searched for and the filters you set, to automatically group matching tasks.")
        }

    }

}
