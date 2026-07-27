/***************************************************************************
//
//    Text button with four intents. One primary action per surface —
//    everything else is "ghost" or "quiet".
//
//    variant: "primary" | "live" | "ghost" | "quiet" | "danger"
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Button {
    id: root

    property string variant: "ghost"
    property string iconName: ""
    property int iconSize: Theme.iconMd

    readonly property bool _filled: variant === "primary" || variant === "live"

    implicitHeight: Theme.ctrlH
    implicitWidth: Math.max(72, row.implicitWidth + Theme.s6 * 2)
    padding: 0
    hoverEnabled: true
    focusPolicy: Qt.TabFocus

    Accessible.name: text
    Accessible.role: Accessible.Button

    ToolTip.visible: hovered && ToolTip.text !== ""
    ToolTip.delay: 550

    readonly property color _base: variant === "primary" ? Theme.primary
                                 : variant === "live" ? Theme.live
                                 : variant === "danger" ? Theme.danger
                                 : "transparent"

    readonly property color _fg: !enabled ? Theme.textDim
                               : _filled ? Theme.textOnAccent
                               : variant === "danger" ? Theme.danger
                               : variant === "quiet" ? Theme.textMuted
                               : Theme.text

    background: Rectangle {
        radius: Theme.rMd
        color: {
            if (!root.enabled) return root._filled ? Theme.overlay : "transparent"
            if (root._filled)
                return root.down ? Qt.darker(root._base, 1.18)
                     : root.hovered ? Qt.lighter(root._base, 1.12)
                     : root._base
            return root.down ? Theme.pressedLayer
                 : root.hovered ? Theme.hoverLayer
                 : "transparent"
        }

        border.width: root.visualFocus ? 2 : (root.variant === "ghost" ? 1 : 0)
        border.color: root.visualFocus ? Theme.primary : Theme.border

        Behavior on color {
            ColorAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
        }
    }

    contentItem: Item {
        Row {
            id: row
            anchors.centerIn: parent
            spacing: Theme.s4

            Icon {
                name: root.iconName
                size: root.iconSize
                color: root._fg
                visible: root.iconName !== ""
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: root.text
                color: root._fg
                font.family: Theme.uiFont
                font.pixelSize: Theme.fsBody
                font.weight: root._filled ? Theme.wSemi : Theme.wMedium
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    scale: down ? 0.975 : 1.0
    Behavior on scale {
        NumberAnimation { duration: Theme.dur(Theme.durInstant); easing.type: Theme.easeOut }
    }

    opacity: enabled ? 1.0 : Theme.disabledOpacity
    Behavior on opacity { NumberAnimation { duration: Theme.dur(Theme.durFast) } }
}
