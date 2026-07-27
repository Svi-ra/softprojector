/***************************************************************************
//
//    Songs module: filter the library on the left, read the selected song's
//    stanzas on the right, send a stanza live.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root

    function focusSearch() { search.forceFieldFocus(); search.selectAllText() }

    Column {
        anchors.fill: parent
        spacing: 0

        Toolbar {
            width: parent.width

            InputField {
                id: search
                width: 260
                anchors.verticalCenter: parent.verticalCenter
                role: "search"
                placeholder: qsTr("Search by title or number…")
                onEdited: App.songQuery = text
                onCleared: App.songQuery = ""
            }

            Segmented {
                anchors.verticalCenter: parent.verticalCenter
                width: 200
                options: [qsTr("Contains"), qsTr("Starts"), qsTr("Exact")]
                currentIndex: App.songMatchMode
                onActivated: function(i) { App.songMatchMode = i }
            }

            ToolSep {}

            Dropdown {
                width: 168
                anchors.verticalCenter: parent.verticalCenter
                iconName: "book-marked"
                label: qsTr("Songbook")
                model: App.songbooks
                currentIndex: App.songbookIndex
                onActivated: App.songbookIndex = currentIndex
            }

            Dropdown {
                width: 180
                anchors.verticalCenter: parent.verticalCenter
                iconName: "funnel"
                label: qsTr("Category")
                model: App.songCategories
                currentIndex: App.songCategoryIndex
                onActivated: App.songCategoryIndex = currentIndex
            }

            ToolSep {}

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "plus"
                text: qsTr("New")
                onClicked: App.invoke("songNew")
            }

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "square-pen"
                text: qsTr("Edit")
                enabled: App.songIndex >= 0
                onClicked: App.invoke("songEdit")
            }

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "copy"
                text: qsTr("Copy")
                enabled: App.songIndex >= 0
                onClicked: App.invoke("songCopy")
            }

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "trash-2"
                text: qsTr("Delete")
                danger: true
                enabled: App.songIndex >= 0
                onClicked: App.invoke("songDelete")
            }
        }

        SplitView {
            width: parent.width
            height: parent.height - Theme.toolbarH
            orientation: Qt.Horizontal
            handle: SplitGrip {}

            Panel {
                SplitView.preferredWidth: 380
                SplitView.minimumWidth: 260
                title: qsTr("Library")
                iconName: "list-music"

                headerItems: [
                    Text {
                        text: qsTr("%n song(s)", "", App.songs.length)
                        color: Theme.textDim
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fsTiny
                        rightPadding: Theme.s4
                        anchors.verticalCenter: parent.verticalCenter
                    }
                ]

                SelectList {
                    anchors.fill: parent
                    model: App.songs
                    currentIndex: App.songIndex
                    rowHeight: Theme.rowHTall
                    emptyIcon: "music-4"
                    emptyTitle: App.songQuery === "" ? qsTr("No songs yet")
                                                     : qsTr("Nothing matches")
                    emptyHint: App.songQuery === ""
                               ? qsTr("Add a songbook from Manage Data, or create a song.")
                               : qsTr("Try a shorter search, or switch the songbook filter to “All”.")
                    emptyAction: App.songQuery === "" ? qsTr("New song") : ""
                    onClicked: function(i) { App.songIndex = i }
                    onActivated: function(i) { App.songIndex = i; App.songGoLive(0) }
                    onEmptyActionTriggered: App.invoke("songNew")
                }
            }

            Panel {
                SplitView.fillWidth: true
                SplitView.minimumWidth: 300
                title: App.songTitle !== "" ? App.songTitle : qsTr("Preview")
                iconName: "music-4"
                accent: Theme.primary

                headerItems: [
                    ToolBtn {
                        iconName: "list-plus"
                        text: qsTr("Schedule")
                        enabled: App.songIndex >= 0
                        onClicked: App.songAddToSchedule()
                    },
                    Btn {
                        text: qsTr("Go Live")
                        iconName: "cast"
                        variant: "live"
                        enabled: App.songIndex >= 0
                        onClicked: App.songGoLive(Math.max(0, stanzas.currentIndex))
                    }
                ]

                Column {
                    anchors.fill: parent
                    spacing: 0

                    // Song metadata strip
                    Item {
                        width: parent.width
                        height: App.songInfo !== "" || App.songNotes !== "" ? meta.height + Theme.s5 * 2 : 0
                        visible: height > 0

                        Column {
                            id: meta
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.leftMargin: Theme.s6
                            anchors.rightMargin: Theme.s6
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: Theme.s3

                            Text {
                                width: parent.width
                                text: App.songInfo
                                visible: text !== ""
                                color: Theme.textDim
                                font.family: Theme.uiFont
                                font.pixelSize: Theme.fsSmall
                                elide: Text.ElideRight
                            }

                            Rectangle {
                                width: parent.width
                                height: notes.height + Theme.s4 * 2
                                visible: App.songNotes !== ""
                                radius: Theme.rMd
                                color: Theme.overlay

                                Row {
                                    anchors.fill: parent
                                    anchors.margins: Theme.s4
                                    spacing: Theme.s4

                                    Icon {
                                        name: "info"
                                        size: Theme.iconSm
                                        color: Theme.info
                                    }

                                    Text {
                                        id: notes
                                        width: parent.width - Theme.iconSm - Theme.s4
                                        text: App.songNotes
                                        color: Theme.textMuted
                                        font.family: Theme.uiFont
                                        font.pixelSize: Theme.fsSmall
                                        wrapMode: Text.WordWrap
                                    }
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

                    SelectList {
                        id: stanzas
                        width: parent.width
                        height: parent.height - (App.songInfo !== "" || App.songNotes !== ""
                                                 ? meta.height + Theme.s5 * 2 : 0)
                        model: App.songStanzas
                        showIcons: false
                        rowHeight: 84
                        wrapText: true
                        cards: true
                        emptyIcon: "music-4"
                        emptyTitle: qsTr("Nothing selected")
                        emptyHint: qsTr("Pick a song on the left to read its stanzas.")
                        onActivated: function(i) { App.songGoLive(i) }
                    }
                }
            }
        }
    }
}
