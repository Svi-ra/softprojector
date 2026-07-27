/***************************************************************************
//
//    SplitView handle. Invisible at rest, a coloured line while being
//    dragged, with a grab area wider than the line it draws.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root

    property bool vertical: false

    // The attached state belongs to the item SplitView assigns as its handle,
    // so read it once here and let the children bind to that.
    readonly property bool grabbed: SplitHandle.pressed
    readonly property bool hovering: SplitHandle.hovered

    implicitWidth: vertical ? 1 : Theme.s3
    implicitHeight: vertical ? Theme.s3 : 1
    color: "transparent"

    Rectangle {
        anchors.centerIn: parent
        width: root.vertical ? parent.width : 1
        height: root.vertical ? 1 : parent.height
        color: root.grabbed ? Theme.primary
             : root.hovering ? Theme.borderStrong
             : Theme.border

        Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durFast) } }
    }

    // Grip dots appear on hover so the handle is discoverable at all.
    Column {
        anchors.centerIn: parent
        spacing: 2
        visible: !root.vertical && (root.hovering || root.grabbed)

        Repeater {
            model: 3
            Rectangle {
                width: 2; height: 2; radius: 1
                color: Theme.textDim
            }
        }
    }

    HoverHandler {
        cursorShape: root.vertical ? Qt.SplitVCursor : Qt.SplitHCursor
    }
}
