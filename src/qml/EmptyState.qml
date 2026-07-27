/***************************************************************************
//
//    Shown wherever a list has nothing in it. An empty region with no
//    explanation is a dead end; this always says what to do next.
//
***************************************************************************/

import QtQuick

Item {
    id: root

    property string iconName: "circle-dot"
    property string title: ""
    property string hint: ""
    property string actionText: ""
    property string actionIcon: ""

    signal actionTriggered()

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - Theme.s8 * 2, 260)
        spacing: Theme.s5

        Rectangle {
            width: 44; height: 44; radius: Theme.rXl
            color: Theme.overlay
            anchors.horizontalCenter: parent.horizontalCenter

            Icon {
                anchors.centerIn: parent
                name: root.iconName
                size: Theme.iconXl
                color: Theme.textDim
            }
        }

        Text {
            width: parent.width
            text: root.title
            visible: root.title !== ""
            color: Theme.textMuted
            font.family: Theme.uiFont
            font.pixelSize: Theme.fsMedium
            font.weight: Theme.wMedium
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            text: root.hint
            visible: root.hint !== ""
            color: Theme.textDim
            font.family: Theme.uiFont
            font.pixelSize: Theme.fsSmall
            lineHeight: 1.45
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Btn {
            text: root.actionText
            iconName: root.actionIcon
            variant: "ghost"
            visible: root.actionText !== ""
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: root.actionTriggered()
        }
    }

    // Fade in so a list that resolves to empty does not flash.
    opacity: 0
    Component.onCompleted: opacity = 1
    Behavior on opacity {
        NumberAnimation { duration: Theme.dur(Theme.durSlow); easing.type: Theme.easeOut }
    }
}
