/***************************************************************************
//
//    Small status chip. Always icon-or-dot + text, never colour alone.
//
//    tone: "neutral" | "primary" | "live" | "danger" | "warning" | "info"
//
***************************************************************************/

import QtQuick

Rectangle {
    id: root

    property string text: ""
    property string iconName: ""
    property string tone: "neutral"
    property bool pulse: false

    readonly property color _fg: tone === "live" ? Theme.live
                               : tone === "primary" ? Theme.primary
                               : tone === "danger" ? Theme.danger
                               : tone === "warning" ? Theme.warning
                               : tone === "info" ? Theme.info
                               : Theme.textMuted

    readonly property color _bg: tone === "live" ? Theme.liveSoft
                               : tone === "primary" ? Theme.primarySoft
                               : tone === "danger" ? Theme.dangerSoft
                               : Theme.overlay

    implicitHeight: 20
    implicitWidth: row.implicitWidth + Theme.s5 * 2
    radius: Theme.rFull
    color: _bg
    border.width: 1
    border.color: Qt.rgba(_fg.r, _fg.g, _fg.b, 0.28)

    Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durFast) } }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: Theme.s3

        Item {
            width: root.iconName !== "" ? Theme.iconSm : 6
            height: width
            anchors.verticalCenter: parent.verticalCenter

            Icon {
                anchors.fill: parent
                name: root.iconName
                size: Theme.iconSm
                color: root._fg
                visible: root.iconName !== ""
            }

            Rectangle {
                anchors.centerIn: parent
                width: 6; height: 6; radius: 3
                color: root._fg
                visible: root.iconName === ""

                SequentialAnimation on opacity {
                    running: root.pulse && root.visible && Theme.dur(1) > 0
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.3; duration: 700; easing.type: Easing.InOutSine }
                    NumberAnimation { to: 1.0; duration: 700; easing.type: Easing.InOutSine }
                }
            }
        }

        Text {
            text: root.text
            color: root._fg
            font.family: Theme.uiFont
            font.pixelSize: Theme.fsTiny
            font.weight: Theme.wSemi
            font.letterSpacing: 0.3
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
