/***************************************************************************
//
//    Media module: the video/audio library. Files can be dropped straight
//    onto the list.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs

Item {
    id: root

    FileDialog {
        id: picker
        title: qsTr("Add media files")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("Media files (*.mp4 *.avi *.mkv *.mov *.wmv *.mpg *.mpeg *.m4v *.webm *.mp3 *.wav *.ogg *.flac *.m4a)"),
                      qsTr("All files (*)")]
        onAccepted: App.mediaAddFiles(selectedFiles)
    }

    Column {
        anchors.fill: parent
        spacing: 0

        Toolbar {
            width: parent.width

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "plus"
                text: qsTr("Add files")
                onClicked: picker.open()
            }

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "trash-2"
                text: qsTr("Remove")
                danger: true
                enabled: App.mediaIndex >= 0
                onClicked: App.mediaRemoveSelected()
            }

            ToolSep {}

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "list-plus"
                text: qsTr("Add to schedule")
                enabled: App.mediaIndex >= 0
                onClicked: App.mediaAddToSchedule()
            }
        }

        Panel {
            width: parent.width
            height: parent.height - Theme.toolbarH
            title: qsTr("Media library")
            iconName: "clapperboard"

            headerItems: [
                Btn {
                    text: qsTr("Go Live")
                    iconName: "cast"
                    variant: "live"
                    enabled: App.mediaIndex >= 0
                    onClicked: App.mediaGoLive()
                }
            ]

            SelectList {
                id: list
                anchors.fill: parent
                model: App.mediaLibrary
                currentIndex: App.mediaIndex
                rowHeight: Theme.rowHTall
                emptyIcon: "clapperboard"
                emptyTitle: qsTr("Library is empty")
                emptyHint: qsTr("Add video or audio files, or drop them onto this panel.")
                emptyAction: qsTr("Add files")
                onClicked: function(i) { App.mediaIndex = i }
                onActivated: function(i) { App.mediaIndex = i; App.mediaGoLive() }
                onEmptyActionTriggered: picker.open()
            }

            DropArea {
                anchors.fill: parent
                onDropped: function(drop) {
                    if (drop.hasUrls) {
                        App.mediaAddFiles(drop.urls)
                        drop.accept()
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: Theme.s4
                    radius: Theme.rLg
                    color: "transparent"
                    border.width: 2
                    border.color: Theme.primary
                    opacity: parent.containsDrag ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: Theme.dur(Theme.durFast) } }

                    Rectangle {
                        anchors.centerIn: parent
                        width: dropLabel.width + Theme.s7 * 2
                        height: 36
                        radius: Theme.rFull
                        color: Theme.primary

                        Text {
                            id: dropLabel
                            anchors.centerIn: parent
                            text: qsTr("Drop to add to the library")
                            color: Theme.textOnAccent
                            font.family: Theme.uiFont
                            font.pixelSize: Theme.fsBody
                            font.weight: Theme.wSemi
                        }
                    }
                }
            }
        }
    }
}
