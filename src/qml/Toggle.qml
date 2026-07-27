/***************************************************************************
//
//    Switch. The knob travels rather than teleports, so the state change
//    reads as one continuous thing.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Switch {
    id: root

    property string label: text

    implicitHeight: Theme.hitMin
    padding: 0
    spacing: Theme.s5
    hoverEnabled: true

    Accessible.name: label
    Accessible.role: Accessible.CheckBox

    indicator: Rectangle {
        implicitWidth: 34
        implicitHeight: 18
        x: 0
        y: (root.height - height) / 2
        radius: height / 2
        color: root.checked ? Theme.primary : Theme.inset
        border.width: 1
        border.color: root.checked ? Theme.primary : Theme.borderStrong

        Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durBase) } }
        Behavior on border.color { ColorAnimation { duration: Theme.dur(Theme.durBase) } }

        Rectangle {
            width: 14; height: 14; radius: 7
            y: (parent.height - height) / 2
            x: root.checked ? parent.width - width - 2 : 2
            color: root.checked ? Theme.textOnAccent : Theme.textMuted

            Behavior on x {
                NumberAnimation { duration: Theme.dur(Theme.durBase); easing.type: Theme.easeEmph }
            }
            Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durBase) } }
        }
    }

    contentItem: Text {
        text: root.text
        visible: root.text !== ""
        color: Theme.text
        font.family: Theme.uiFont
        font.pixelSize: Theme.fsBody
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.indicator.width + root.spacing
    }

    opacity: enabled ? 1.0 : Theme.disabledOpacity
}
