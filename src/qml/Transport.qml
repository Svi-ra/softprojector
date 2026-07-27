/***************************************************************************
//
//    Media transport: play/pause, stop, scrub, volume, elapsed/total.
//    Times use tabular figures so the row does not twitch every second.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root

    property bool playing: false
    property real position: 0        // ms
    property real duration: 0        // ms
    property int volume: 100
    property bool muted: false

    signal playPause()
    signal stopped()
    signal seek(real ms)
    signal volumeSet(int level)
    signal muteToggled(bool on)

    function fmt(ms) {
        if (!(ms > 0)) return "0:00"
        var t = Math.floor(ms / 1000)
        var h = Math.floor(t / 3600)
        var m = Math.floor((t % 3600) / 60)
        var s = t % 60
        return (h > 0 ? h + ":" + (m < 10 ? "0" : "") : "") + m + ":" + (s < 10 ? "0" : "") + s
    }

    implicitHeight: 64
    color: Theme.inset
    radius: Theme.rLg
    border.width: 1
    border.color: Theme.border

    Column {
        anchors.fill: parent
        anchors.margins: Theme.s5
        spacing: Theme.s2

        // ── Scrubber ────────────────────────────────────────────────────────
        Item {
            width: parent.width
            height: 18

            SliderBar {
                id: scrub
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 44
                anchors.rightMargin: 44
                anchors.verticalCenter: parent.verticalCenter
                height: 18
                accent: Theme.live
                label: qsTr("Playback position")
                from: 0
                to: Math.max(1, root.duration)
                // Do not fight the user's drag with incoming position updates.
                value: pressed ? value : root.position
                onMoved: root.seek(value)
            }

            Text {
                text: root.fmt(root.position)
                color: Theme.textMuted
                font.family: Theme.monoFont
                font.pixelSize: Theme.fsTiny
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: root.fmt(root.duration)
                color: Theme.textDim
                font.family: Theme.monoFont
                font.pixelSize: Theme.fsTiny
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // ── Buttons ─────────────────────────────────────────────────────────
        Item {
            width: parent.width
            height: Theme.hitMin

            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.s2

                IconButton {
                    iconName: root.playing ? "pause" : "play"
                    iconColor: Theme.live
                    activeColor: Theme.live
                    iconSize: Theme.iconLg
                    ToolTip.text: root.playing ? qsTr("Pause") : qsTr("Play")
                    onClicked: root.playPause()
                }

                IconButton {
                    iconName: "square"
                    ToolTip.text: qsTr("Stop")
                    onClicked: root.stopped()
                }
            }

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.s3

                IconButton {
                    iconName: root.muted || root.volume === 0 ? "volume-x" : "volume-2"
                    danger: root.muted
                    ToolTip.text: root.muted ? qsTr("Unmute") : qsTr("Mute")
                    anchors.verticalCenter: parent.verticalCenter
                    onClicked: root.muteToggled(!root.muted)
                }

                SliderBar {
                    width: 82
                    height: Theme.hitMin
                    anchors.verticalCenter: parent.verticalCenter
                    label: qsTr("Volume")
                    from: 0
                    to: 100
                    value: root.muted ? 0 : root.volume
                    enabled: !root.muted
                    onMoved: root.volumeSet(Math.round(value))
                }
            }
        }
    }
}
