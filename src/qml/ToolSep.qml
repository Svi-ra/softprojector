/***************************************************************************
//
//    Vertical hairline between toolbar groups.
//
***************************************************************************/

import QtQuick

Item {
    implicitWidth: Theme.s5
    implicitHeight: Theme.ctrlH

    Rectangle {
        anchors.centerIn: parent
        width: 1
        height: parent.height - Theme.s4 * 2
        color: Theme.border
    }
}
