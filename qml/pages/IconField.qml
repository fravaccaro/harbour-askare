/*!
    \qmltype IconField
    \brief Frames a value control the way IconMenuItem frames a value.

    A leading theme icon, the control itself, and a caption below it, on the
    margins IconMenuItem uses, so a slider or a progress bar lines up with the
    rows around it. The component holds no control of its own: the caller
    places one inside and gives it \c grooveLeftMargin and \c grooveRightMargin
    so its groove runs from the value column to the page margin. Leave the
    control's own \c label unset, as it would centre itself under the groove;
    \c description draws the caption instead.

    The control is inset to a padding short of the value column rather than
    spanning the page, because the value label a Slider floats above its handle
    clamps to the control's own left edge: given the whole width it would climb
    over the icon.

    The icon and the caption sit around \c lineCenterY, which defaults to the
    middle of the control. A Silica Slider carrying a \c valueText reserves an
    ascent above its groove and only half an item below it, so it names
    \c {height - Theme.itemSizeSmall / 2}; a ProgressBar names its
    \c barCenterY.

    \qml
    IconField {
        id: field

        width: parent.width
        description: qsTr("Completion")
        icon.source: "image://theme/icon-m-acknowledge"
        lineCenterY: bar.barCenterY

        ProgressBar {
            id: bar

            width: parent.width
            leftMargin: field.grooveLeftMargin
            rightMargin: field.grooveRightMargin
            value: 40
        }
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: root

    property alias icon: iconImage
    property string description
    property bool highlighted
    property real lineCenterY: contentItem.height / 2
    // Where the value of a row begins, the icon and its padding apart.
    readonly property real contentLeft: Theme.horizontalPageMargin + Theme.iconSizeMedium + Theme.paddingMedium
    readonly property real grooveLeftMargin: Theme.paddingMedium
    readonly property real grooveRightMargin: Theme.horizontalPageMargin
    default property alias _content: contentItem.data

    width: parent ? parent.width : Screen.width
    // The caption closes the row: a control reserves room under its groove for
    // the label it is not drawing, and that empty room would pad the row twice.
    height: caption.visible ? caption.y + caption.height : contentItem.height

    HighlightImage {
        id: iconImage

        x: Theme.horizontalPageMargin
        y: root.lineCenterY - height / 2
        sourceSize.width: Theme.iconSizeMedium
        sourceSize.height: Theme.iconSizeMedium
        highlighted: root.highlighted
    }

    Item {
        id: contentItem

        x: root.contentLeft - Theme.paddingMedium
        width: parent.width - x
        height: childrenRect.height
    }

    Label {
        id: caption

        x: root.contentLeft
        y: root.lineCenterY + Theme.paddingSmall
        width: parent.width - x - Theme.horizontalPageMargin
        visible: text.length
        color: Theme.rgba(root.highlighted ? Theme.highlightColor : Theme.primaryColor, Theme.opacityHigh)
        font.pixelSize: Theme.fontSizeExtraSmall
        truncationMode: TruncationMode.Fade
        text: root.description
    }

}
