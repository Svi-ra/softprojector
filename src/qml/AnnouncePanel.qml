/***************************************************************************
//
//    Announcements module.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root

    Column {
        anchors.fill: parent
        spacing: 0

        Toolbar {
            width: parent.width

            InputField {
                width: 260
                anchors.verticalCenter: parent.verticalCenter
                role: "search"
                placeholder: qsTr("Search announcements…")
                onEdited: App.announceQuery = text
                onCleared: App.announceQuery = ""
            }

            ToolSep {}

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "plus"
                text: qsTr("New")
                onClicked: App.invoke("announceNew")
            }

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "square-pen"
                text: qsTr("Edit")
                enabled: App.announceIndex >= 0
                onClicked: App.invoke("announceEdit")
            }

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "copy"
                text: qsTr("Copy")
                enabled: App.announceIndex >= 0
                onClicked: App.invoke("announceCopy")
            }

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "trash-2"
                text: qsTr("Delete")
                danger: true
                enabled: App.announceIndex >= 0
                onClicked: App.invoke("announceDelete")
            }
        }

        SplitView {
            width: parent.width
            height: parent.height - Theme.toolbarH
            orientation: Qt.Horizontal
            handle: SplitGrip {}

            Panel {
                SplitView.preferredWidth: 360
                SplitView.minimumWidth: 240
                title: qsTr("Announcements")
                iconName: "megaphone"

                SelectList {
                    anchors.fill: parent
                    model: App.announcements
                    currentIndex: App.announceIndex
                    rowHeight: Theme.rowHTall
                    emptyIcon: "megaphone"
                    emptyTitle: qsTr("No announcements")
                    emptyHint: qsTr("Create one to show notices between songs.")
                    emptyAction: qsTr("New announcement")
                    onClicked: function(i) { App.announceIndex = i }
                    onActivated: function(i) { App.announceIndex = i; App.announceGoLive(0) }
                    onEmptyActionTriggered: App.invoke("announceNew")
                }
            }

            Panel {
                SplitView.fillWidth: true
                SplitView.minimumWidth: 300
                title: App.announceTitle !== "" ? App.announceTitle : qsTr("Preview")
                iconName: "megaphone"
                accent: Theme.primary

                headerItems: [
                    ToolBtn {
                        iconName: "list-plus"
                        text: qsTr("Schedule")
                        enabled: App.announceIndex >= 0
                        onClicked: App.announceAddToSchedule()
                    },
                    Btn {
                        text: qsTr("Go Live")
                        iconName: "cast"
                        variant: "live"
                        enabled: App.announceIndex >= 0
                        onClicked: App.announceGoLive(Math.max(0, slides.currentIndex))
                    }
                ]

                SelectList {
                    id: slides
                    anchors.fill: parent
                    model: App.announceSlides
                    showIcons: false
                    rowHeight: 84
                    wrapText: true
                    cards: true
                    emptyIcon: "megaphone"
                    emptyTitle: qsTr("Nothing selected")
                    emptyHint: qsTr("Pick an announcement to see its slides.")
                    onActivated: function(i) { App.announceGoLive(i) }
                }
            }
        }
    }
}
