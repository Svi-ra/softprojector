/***************************************************************************
//
//    The live column: what is on the projector right now, the slides you can
//    step through, and the controls that put it there or take it away.
//
//    This is the highest-stakes region of the app, so it is the only place
//    that uses the live green, and the only place with a persistent status
//    indicator.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Panel {
    id: root

    property alias currentIndex: slides.currentIndex

    title: qsTr("Live")
    iconName: "cast"
    accent: App.live ? Theme.live : Theme.textDim

    headerItems: [
        Badge {
            text: App.live ? qsTr("On air") : qsTr("Idle")
            tone: App.live ? "live" : "neutral"
            pulse: App.live
            anchors.verticalCenter: parent.verticalCenter
        }
    ]

    Column {
        anchors.fill: parent
        spacing: 0

        // ── What is showing ─────────────────────────────────────────────────
        Item {
            width: parent.width
            height: App.live ? 52 : 0
            visible: App.live
            clip: true

            Behavior on height {
                NumberAnimation { duration: Theme.dur(Theme.durBase); easing.type: Theme.easeOut }
            }

            Row {
                anchors.fill: parent
                anchors.leftMargin: Theme.s6
                anchors.rightMargin: Theme.s6
                spacing: Theme.s5

                Rectangle {
                    width: 32; height: 32; radius: Theme.rLg
                    color: Theme.liveSoft
                    anchors.verticalCenter: parent.verticalCenter

                    Icon {
                        anchors.centerIn: parent
                        name: App.liveIcon
                        size: Theme.iconMd
                        color: Theme.live
                    }
                }

                Column {
                    width: parent.width - 32 - Theme.s5
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 1

                    Text {
                        width: parent.width
                        text: App.liveTitle
                        color: Theme.text
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fsMedium
                        font.weight: Theme.wSemi
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: App.liveSlides.length > 0
                              ? qsTr("Slide %1 of %2").arg(App.liveIndex + 1).arg(App.liveSlides.length)
                              : ""
                        visible: text !== ""
                        color: Theme.textDim
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fsTiny
                    }
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: Theme.divider
            }
        }

        // ── Multi-verse toggle (Bible only) ─────────────────────────────────
        Item {
            width: parent.width
            height: App.liveKind === "bible" ? Theme.toolbarH : 0
            visible: App.liveKind === "bible"
            clip: true

            Behavior on height {
                NumberAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
            }

            Toggle {
                anchors.left: parent.left
                anchors.leftMargin: Theme.s6
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Show several verses at once")
                checked: App.multiVerse
                onToggled: App.multiVerse = checked
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: Theme.divider
            }
        }

        // ── Slides ──────────────────────────────────────────────────────────
        SelectList {
            id: slides
            width: parent.width
            height: parent.height
                    - (App.live ? 52 : 0)
                    - (App.liveKind === "bible" ? Theme.toolbarH : 0)
                    - (App.liveKind === "video" ? transport.height + Theme.s5 : 0)
                    - notes.height
                    - controls.height

            model: App.liveSlides
            showIcons: false
            multiSelect: App.multiVerse
            currentIndex: App.liveIndex
            liveMarker: true
            liveRow: App.liveIndex
            rowHeight: App.liveKind === "bible" ? 62 : 78
            wrapText: true
            cards: App.liveKind !== "bible"

            emptyIcon: "monitor"
            emptyTitle: qsTr("Nothing is live")
            emptyHint: qsTr("Send a verse, song or slide from the workspace and it will appear here, ready to step through.")

            // Clicking a slide sends it straight to the projector — that is
            // the whole job of this column. In multi-verse mode the operator
            // picks a range first and confirms with Show, so nothing is
            // pushed until then.
            onClicked: function(i) {
                App.liveIndex = i
                if (App.multiVerse)
                    App.setLiveRows(selection)
                else
                    App.invoke("refreshScreen")
            }
        }

        // ── Song notes ──────────────────────────────────────────────────────
        Item {
            id: notes
            width: parent.width
            height: App.liveNotes !== "" ? notesText.height + Theme.s5 * 2 : 0
            visible: App.liveNotes !== ""

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: Theme.divider
            }

            Row {
                anchors.fill: parent
                anchors.margins: Theme.s5
                spacing: Theme.s4

                Icon { name: "info"; size: Theme.iconSm; color: Theme.info }

                Text {
                    id: notesText
                    width: parent.width - Theme.iconSm - Theme.s4
                    text: App.liveNotes
                    color: Theme.textMuted
                    font.family: Theme.uiFont
                    font.pixelSize: Theme.fsSmall
                    wrapMode: Text.WordWrap
                }
            }
        }

        // ── Media transport ─────────────────────────────────────────────────
        Item {
            width: parent.width
            height: App.liveKind === "video" ? transport.height + Theme.s5 : 0
            visible: App.liveKind === "video"

            Transport {
                id: transport
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: Theme.s5
                anchors.verticalCenter: parent.verticalCenter

                playing: App.mediaPlaying
                position: App.mediaPosition
                duration: App.mediaDuration
                volume: App.mediaVolume
                muted: App.mediaMuted

                onPlayPause: App.invoke("mediaPlayPause")
                onStopped: App.invoke("mediaStop")
                onSeek: function(ms) { App.mediaSeek(Math.round(ms)) }
                onVolumeSet: function(v) { App.mediaVolume = v }
                onMuteToggled: function(on) { App.mediaMuted = on }
            }
        }

        // ── Show / Hide / Clear ─────────────────────────────────────────────
        Rectangle {
            id: controls
            width: parent.width
            height: 96
            color: Theme.inset

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: Theme.divider
            }

            Column {
                anchors.fill: parent
                anchors.margins: Theme.s5
                spacing: Theme.s4

                // Step controls
                Row {
                    width: parent.width
                    spacing: Theme.s4

                    Btn {
                        width: (parent.width - Theme.s4) / 2
                        text: qsTr("Previous")
                        iconName: "chevron-left"
                        variant: "ghost"
                        enabled: App.live && App.liveIndex > 0
                        onClicked: App.invoke("prevSlide")
                    }

                    Btn {
                        width: (parent.width - Theme.s4) / 2
                        text: qsTr("Next")
                        iconName: "chevron-right"
                        variant: "ghost"
                        enabled: App.live && App.liveIndex < App.liveSlides.length - 1
                        onClicked: App.invoke("nextSlide")
                    }
                }

                // Primary action row
                Row {
                    id: primaryRow
                    width: parent.width
                    spacing: Theme.s4

                    // Two equal buttons plus a fixed-width clear affordance.
                    readonly property real halfWidth:
                        (width - Theme.ctrlH - spacing * 2) / 2

                    Btn {
                        width: primaryRow.halfWidth
                        text: qsTr("Show")
                        iconName: "cast"
                        variant: "live"
                        enabled: App.canShow
                        ToolTip.text: qsTr("Shift+F5")
                        onClicked: App.invoke("show")
                    }

                    Btn {
                        width: primaryRow.halfWidth
                        text: qsTr("Hide")
                        iconName: "eye-off"
                        variant: "ghost"
                        enabled: App.canHide
                        onClicked: App.invoke("hide")
                    }

                    IconButton {
                        iconName: "eraser"
                        iconSize: Theme.iconMd
                        width: Theme.ctrlH
                        height: Theme.ctrlH
                        danger: true
                        enabled: App.canClear
                        ToolTip.text: qsTr("Clear the text but keep the background")
                        onClicked: App.invoke("clear")
                    }
                }
            }
        }
    }
}
