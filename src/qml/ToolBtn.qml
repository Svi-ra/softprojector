/***************************************************************************
//
//    Toolbar button: icon plus optional label. Used across every toolbar so
//    density and hit area stay identical throughout the workspace.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Button {
    id: root

    property string iconName: ""
    property bool showLabel: true
    property bool danger: false
    property bool accent: false
    property int iconSize: Theme.iconMd

    implicitHeight: Theme.ctrlH
    implicitWidth: row.implicitWidth + (showLabel ? Theme.s5 * 2 : Theme.s4 * 2)
    padding: 0
    hoverEnabled: true
    focusPolicy: Qt.TabFocus

    Accessible.name: text
    Accessible.role: Accessible.Button

    ToolTip.visible: hovered && ToolTip.text !== ""
    ToolTip.delay: 550

    readonly property color _tint: !enabled ? Theme.textDim
                                 : danger && (hovered || down) ? Theme.danger
                                 : accent ? Theme.live
                                 : checked ? Theme.primary
                                 : hovered || down ? Theme.text
                                 : Theme.textMuted

    background: Rectangle {
        radius: Theme.rMd
        color: root.down ? Theme.pressedLayer
             : root.checked ? Theme.selectedLayer
             : root.hovered ? Theme.hoverLayer
             : "transparent"
        border.width: root.visualFocus ? 2 : 0
        border.color: Theme.primary

        Behavior on color {
            ColorAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
        }
    }

    contentItem: Item {
        Row {
            id: row
            anchors.centerIn: parent
            spacing: Theme.s3

            Icon {
                name: root.iconName
                size: root.iconSize
                color: root._tint
                anchors.verticalCenter: parent.verticalCenter
                visible: root.iconName !== ""
            }

            Text {
                text: root.text
                visible: root.showLabel && root.text !== ""
                color: root._tint
                font.family: Theme.uiFont
                font.pixelSize: Theme.fsBody
                font.weight: Theme.wMedium
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    opacity: enabled ? 1.0 : Theme.disabledOpacity
    Behavior on opacity { NumberAnimation { duration: Theme.dur(Theme.durFast) } }
}
