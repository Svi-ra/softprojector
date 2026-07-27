/***************************************************************************
//
//    Slim scrollbar that thickens on hover. Present but never loud.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

ScrollBar {
    id: root

    policy: ScrollBar.AsNeeded
    minimumSize: 0.08

    contentItem: Rectangle {
        implicitWidth: root.hovered || root.pressed ? 8 : 4
        implicitHeight: root.hovered || root.pressed ? 8 : 4
        radius: width / 2
        color: root.pressed ? Theme.textMuted : Theme.borderStrong
        opacity: root.policy === ScrollBar.AlwaysOn || root.active ? 1.0 : 0.0

        Behavior on implicitWidth {
            NumberAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
        }
        Behavior on implicitHeight {
            NumberAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
        }
        Behavior on opacity { NumberAnimation { duration: Theme.dur(Theme.durBase) } }
        Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durFast) } }
    }

    background: null
}
