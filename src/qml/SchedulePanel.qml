/***************************************************************************
//
//    The service schedule: the running order for the meeting. Always visible
//    on the left, because it is the thing the operator works down.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Panel {
    id: root

    title: qsTr("Schedule")
    iconName: "calendar-days"

    headerItems: [
        Badge {
            text: App.scheduleDirty ? qsTr("Unsaved") : qsTr("Saved")
            tone: App.scheduleDirty ? "warning" : "neutral"
            visible: App.schedule.length > 0
            anchors.verticalCenter: parent.verticalCenter
        },
        IconButton {
            iconName: "ellipsis"
            ToolTip.text: qsTr("Schedule actions")
            onClicked: menu.open()

            Menu {
                id: menu
                y: parent.height + Theme.s2
                implicitWidth: 200
                padding: Theme.s2

                background: Rectangle {
                    color: Theme.overlay
                    radius: Theme.rLg
                    border.width: 1
                    border.color: Theme.borderStrong
                }

                Repeater {
                    model: [
                        { text: qsTr("New schedule"),  icon: "file-plus",  action: "newSchedule" },
                        { text: qsTr("Open…"),         icon: "folder-open", action: "openSchedule" },
                        { text: qsTr("Save"),          icon: "save",       action: "saveSchedule" },
                        { text: qsTr("Save as…"),      icon: "save",       action: "saveScheduleAs" },
                        { text: qsTr("Print schedule"), icon: "printer",   action: "printSchedule" },
                        { text: qsTr("Clear all"),     icon: "list-x",     action: "scheduleClear" }
                    ]

                    delegate: MenuItem {
                        required property var modelData
                        height: Theme.rowH
                        padding: 0
                        onTriggered: App.invoke(modelData.action)

                        background: Rectangle {
                            radius: Theme.rSm
                            color: parent.highlighted ? Theme.hoverLayer : "transparent"
                        }

                        contentItem: Row {
                            spacing: Theme.s4
                            leftPadding: Theme.s5

                            Icon {
                                name: modelData.icon
                                size: Theme.iconSm
                                color: Theme.textMuted
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: modelData.text
                                color: Theme.text
                                font.family: Theme.uiFont
                                font.pixelSize: Theme.fsBody
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }
        }
    ]

    Column {
        anchors.fill: parent
        spacing: 0

        SelectList {
            id: list
            width: parent.width
            height: parent.height - reorder.height
            model: App.schedule
            currentIndex: App.scheduleIndex
            rowHeight: Theme.rowHTall
            emptyIcon: "calendar-days"
            emptyTitle: qsTr("Nothing scheduled")
            emptyHint: qsTr("Add songs, readings and media as you plan the meeting; they queue up here in order.")
            onClicked: function(i) { App.scheduleIndex = i }
            onActivated: function(i) { App.scheduleActivate(i) }
        }

        // Reorder / remove strip. Disabled rather than hidden so the controls
        // do not jump around as the list fills.
        Rectangle {
            id: reorder
            width: parent.width
            height: Theme.toolbarH
            color: "transparent"

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: Theme.divider
            }

            readonly property bool has: App.scheduleIndex >= 0
            readonly property int last: App.schedule.length - 1

            Row {
                anchors.left: parent.left
                anchors.leftMargin: Theme.s4
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.s1

                IconButton {
                    iconName: "chevrons-up"
                    ToolTip.text: qsTr("Move to top")
                    enabled: reorder.has && App.scheduleIndex > 0
                    onClicked: App.scheduleMove(App.scheduleIndex, 0)
                }
                IconButton {
                    iconName: "chevron-up"
                    ToolTip.text: qsTr("Move up")
                    enabled: reorder.has && App.scheduleIndex > 0
                    onClicked: App.scheduleMove(App.scheduleIndex, App.scheduleIndex - 1)
                }
                IconButton {
                    iconName: "chevron-down"
                    ToolTip.text: qsTr("Move down")
                    enabled: reorder.has && App.scheduleIndex < reorder.last
                    onClicked: App.scheduleMove(App.scheduleIndex, App.scheduleIndex + 1)
                }
                IconButton {
                    iconName: "chevrons-down"
                    ToolTip.text: qsTr("Move to bottom")
                    enabled: reorder.has && App.scheduleIndex < reorder.last
                    onClicked: App.scheduleMove(App.scheduleIndex, reorder.last)
                }
            }

            IconButton {
                iconName: "list-x"
                danger: true
                ToolTip.text: qsTr("Remove from schedule")
                enabled: reorder.has
                anchors.right: parent.right
                anchors.rightMargin: Theme.s4
                anchors.verticalCenter: parent.verticalCenter
                onClicked: App.invoke("scheduleRemove")
            }
        }
    }
}
