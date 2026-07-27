/***************************************************************************
//
//    Horizontal toolbar strip. Children are laid out left to right; put a
//    ToolbarSpacer between groups to push the rest to the right edge.
//
***************************************************************************/

import QtQuick

Rectangle {
    id: root

    default property alias items: row.data
    property alias spacing: row.spacing
    property bool bordered: true

    implicitHeight: Theme.toolbarH
    color: "transparent"

    Row {
        id: row
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Theme.s4
        anchors.rightMargin: Theme.s4
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.s1
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: Theme.divider
        visible: root.bordered
    }
}
