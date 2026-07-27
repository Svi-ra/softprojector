/***************************************************************************
//
//    The row used by every list in the workspace: schedule, search results,
//    songs, media library. Selection is a left rail plus a tint — colour
//    alone never carries the state.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

ItemDelegate {
    id: root

    property string iconName: ""
    property color iconColor: Theme.textDim
    property string secondaryText: ""
    property string leadingText: ""
    property bool wrap: false
    // Slides read as discrete blocks; plain rows read as a continuous text.
    property bool card: false
    property string trailingText: ""
    property bool selected: false
    property bool isLive: false
    property bool monospaceTrailing: true

    // Hover visuals can be driven externally when a parent owns the mouse.
    property bool hovering: hovered
    property alias trailingItems: trailing.data

    height: Theme.rowH
    padding: 0
    hoverEnabled: true

    Accessible.name: text
    Accessible.role: Accessible.ListItem
    Accessible.selected: selected

    background: Rectangle {
        color: root.selected ? Theme.selectedLayer
             : root.down ? Theme.pressedLayer
             : root.hovering ? Theme.hoverLayer
             : root.card ? Theme.inset
             : "transparent"
        radius: root.card ? Theme.rMd : Theme.rSm
        border.width: root.card ? 1 : 0
        border.color: root.selected ? Theme.primary : Theme.border

        Behavior on color {
            ColorAnimation { duration: Theme.dur(Theme.durInstant); easing.type: Theme.easeOut }
        }

        // Selection rail. Grows from nothing so the eye tracks the move.
        Rectangle {
            width: 3
            height: root.selected ? parent.height - Theme.s3 : 0
            radius: 2
            color: root.isLive ? Theme.live : Theme.primary
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            Behavior on height {
                NumberAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
            }
        }
    }

    contentItem: Item {
        Icon {
            id: lead
            name: root.iconName
            size: Theme.iconSm
            color: root.isLive ? Theme.live
                 : root.selected ? Theme.primary
                 : root.iconColor
            visible: root.iconName !== ""
            anchors.left: parent.left
            anchors.leftMargin: Theme.s5
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            id: leadNum
            text: root.leadingText
            visible: root.leadingText !== ""
            width: visible ? 22 : 0
            color: root.selected ? Theme.primary : Theme.textDim
            font.family: Theme.monoFont
            font.pixelSize: Theme.fsTiny
            horizontalAlignment: Text.AlignRight
            anchors.left: lead.visible ? lead.right : parent.left
            anchors.leftMargin: lead.visible ? Theme.s4 : Theme.s5
            anchors.top: parent.top
            anchors.topMargin: Theme.s4
        }

        Column {
            anchors.left: leadNum.visible ? leadNum.right : (lead.visible ? lead.right : parent.left)
            anchors.leftMargin: leadNum.visible ? Theme.s4 : (lead.visible ? Theme.s4 : Theme.s5)
            anchors.right: trailing.left
            anchors.rightMargin: Theme.s4
            anchors.verticalCenter: root.wrap ? undefined : parent.verticalCenter
            anchors.top: root.wrap ? parent.top : undefined
            anchors.topMargin: root.wrap ? Theme.s5 : 0
            spacing: 2

            Text {
                width: parent.width
                text: root.text
                color: root.selected || root.hovering ? Theme.text : Theme.textMuted
                font.family: Theme.uiFont
                font.pixelSize: Theme.fsBody
                font.weight: root.selected ? Theme.wMedium : Theme.wRegular
                lineHeight: root.wrap ? 1.25 : 1.0
                wrapMode: root.wrap ? Text.WordWrap : Text.NoWrap
                maximumLineCount: root.wrap ? 3 : 1
                elide: Text.ElideRight

                Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durInstant) } }
            }

            Text {
                width: parent.width
                text: root.secondaryText
                visible: root.secondaryText !== ""
                color: Theme.textDim
                font.family: Theme.uiFont
                font.pixelSize: Theme.fsTiny
                elide: Text.ElideRight
            }
        }

        Row {
            id: trailing
            anchors.right: parent.right
            anchors.rightMargin: Theme.s4
            anchors.verticalCenter: root.wrap ? undefined : parent.verticalCenter
            anchors.top: root.wrap ? parent.top : undefined
            anchors.topMargin: root.wrap ? Theme.s5 : 0
            spacing: Theme.s3

            Text {
                text: root.trailingText
                visible: root.trailingText !== ""
                color: Theme.textDim
                font.family: root.monospaceTrailing ? Theme.monoFont : Theme.uiFont
                font.pixelSize: Theme.fsTiny
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
