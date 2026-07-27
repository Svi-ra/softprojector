/***************************************************************************
//
//    Transient messages, bottom-centre. They never steal focus and never
//    block a control, because they can appear mid-service.
//
***************************************************************************/

import QtQuick

Item {
    id: root

    property int lifetime: 4000

    function show(message, tone) {
        toast.message = message
        toast.tone = tone || "neutral"
        toast.shown = true
        timer.restart()
    }

    Rectangle {
        id: toast

        property string message: ""
        property string tone: "neutral"
        property bool shown: false

        readonly property color _accent: tone === "live" ? Theme.live
                                       : tone === "danger" ? Theme.danger
                                       : tone === "warning" ? Theme.warning
                                       : Theme.info

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: shown ? Theme.s9 : 0

        width: content.implicitWidth + Theme.s7 * 2
        height: 40
        radius: Theme.rLg
        color: Theme.raised
        border.width: 1
        border.color: Theme.borderStrong

        opacity: shown ? 1 : 0
        visible: opacity > 0

        Accessible.role: Accessible.StaticText
        Accessible.name: message

        Behavior on opacity {
            NumberAnimation { duration: Theme.dur(Theme.durBase); easing.type: Theme.easeOut }
        }
        Behavior on anchors.bottomMargin {
            NumberAnimation { duration: Theme.dur(Theme.durBase); easing.type: Theme.easeEmph }
        }

        Row {
            id: content
            anchors.centerIn: parent
            spacing: Theme.s4

            Rectangle {
                width: 6; height: 6; radius: 3
                color: toast._accent
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: toast.message
                color: Theme.text
                font.family: Theme.uiFont
                font.pixelSize: Theme.fsBody
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Timer {
            id: timer
            interval: root.lifetime
            onTriggered: toast.shown = false
        }
    }
}
