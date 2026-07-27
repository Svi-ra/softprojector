/***************************************************************************
//
//    Slider used for media position and volume. The handle only appears on
//    hover or drag so a resting transport bar stays quiet, but the track is
//    always thick enough to grab.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Slider {
    id: root

    property string label: ""
    property color accent: Theme.primary

    implicitHeight: Theme.hitMin
    padding: 0

    Accessible.name: label
    Accessible.role: Accessible.Slider

    background: Rectangle {
        x: 0
        y: (root.height - height) / 2
        width: root.availableWidth
        height: root.hovered || root.pressed ? 6 : 4
        radius: height / 2
        color: Theme.inset

        Behavior on height {
            NumberAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
        }

        Rectangle {
            width: root.visualPosition * parent.width
            height: parent.height
            radius: parent.radius
            color: root.accent
        }
    }

    handle: Rectangle {
        x: root.visualPosition * (root.availableWidth - width)
        y: (root.height - height) / 2
        width: 12
        height: 12
        radius: 6
        color: Theme.text
        border.width: 2
        border.color: root.accent

        scale: root.pressed ? 1.15 : (root.hovered ? 1.0 : 0.0)
        Behavior on scale {
            NumberAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
        }
    }

    opacity: enabled ? 1.0 : Theme.disabledOpacity
}
