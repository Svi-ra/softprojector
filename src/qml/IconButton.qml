/***************************************************************************
//
//    Square icon-only button. Always carries an accessible name via its
//    tooltip text — an unlabelled glyph is not a control.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Button {
    id: root

    property string iconName: ""
    property color iconColor: Theme.textMuted
    property color activeColor: Theme.text
    property int iconSize: Theme.iconMd
    property bool danger: false
    property bool accent: false

    implicitWidth: Theme.hitMin
    implicitHeight: Theme.hitMin
    padding: 0
    hoverEnabled: true
    focusPolicy: Qt.TabFocus

    Accessible.name: text !== "" ? text : ToolTip.text
    Accessible.role: Accessible.Button

    ToolTip.visible: hovered && ToolTip.text !== ""
    ToolTip.delay: 550

    readonly property color _tint: !enabled ? Theme.textDim
                                 : danger && (hovered || down) ? Theme.danger
                                 : accent ? Theme.live
                                 : checked ? Theme.primary
                                 : hovered || down ? activeColor
                                 : iconColor

    background: Rectangle {
        radius: Theme.rMd
        color: root.down ? (root.danger ? Theme.dangerSoft : Theme.pressedLayer)
             : root.checked ? Theme.selectedLayer
             : root.hovered ? (root.danger ? Theme.dangerSoft : Theme.hoverLayer)
             : "transparent"

        border.width: root.visualFocus ? 2 : 0
        border.color: Theme.primary

        Behavior on color {
            ColorAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
        }
    }

    contentItem: Icon {
        name: root.iconName
        size: root.iconSize
        color: root._tint
        anchors.centerIn: parent

        // Scale feedback only — never a geometry change, which would nudge
        // everything around it.
        scale: root.down ? 0.88 : 1.0
        Behavior on scale {
            NumberAnimation { duration: Theme.dur(Theme.durInstant); easing.type: Theme.easeOut }
        }
    }

    opacity: enabled ? 1.0 : Theme.disabledOpacity
    Behavior on opacity {
        NumberAnimation { duration: Theme.dur(Theme.durFast) }
    }
}
