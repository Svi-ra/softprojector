/***************************************************************************
//
//    Application menu bar rendered in the title strip.
//
//    menus: [{ title, items: [{ text, shortcut, icon, action, enabled,
//                               checkable, checked, separator }] }]
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Row {
    id: root

    property var menus: []
    signal triggered(string action, bool checked)

    spacing: 0

    Repeater {
        model: root.menus

        delegate: Item {
            id: entry
            required property var modelData
            required property int index

            width: label.implicitWidth + Theme.s5 * 2
            height: Theme.titleBarH - Theme.s3

            Rectangle {
                anchors.fill: parent
                radius: Theme.rSm
                color: menu.visible ? Theme.selectedLayer
                     : ma.containsMouse ? Theme.hoverLayer
                     : "transparent"
                Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durInstant) } }
            }

            Text {
                id: label
                anchors.centerIn: parent
                text: entry.modelData.title
                color: menu.visible || ma.containsMouse ? Theme.text : Theme.textMuted
                font.family: Theme.uiFont
                font.pixelSize: Theme.fsBody
                font.weight: Theme.wMedium
                Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durInstant) } }
            }

            MouseArea {
                id: ma
                anchors.fill: parent
                hoverEnabled: true
                onClicked: menu.visible ? menu.close() : menu.open()
                // Once one menu is open, hovering the next opens it: the
                // standard desktop behaviour people already have in muscle.
                onEntered: if (root._anyOpen && !menu.visible) menu.open()
            }

            Menu {
                id: menu
                y: entry.height + Theme.s2
                implicitWidth: 240
                padding: Theme.s2

                onOpenedChanged: root._anyOpen = opened

                background: Rectangle {
                    color: Theme.overlay
                    radius: Theme.rLg
                    border.width: 1
                    border.color: Theme.borderStrong
                }

                enter: Transition {
                    NumberAnimation { property: "opacity"; from: 0; to: 1
                                      duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
                    NumberAnimation { property: "scale"; from: 0.97; to: 1
                                      duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
                }
                exit: Transition {
                    NumberAnimation { property: "opacity"; from: 1; to: 0
                                      duration: Theme.dur(Theme.durInstant) }
                }

                Repeater {
                    model: entry.modelData.items

                    delegate: MenuItem {
                        id: mi
                        required property var modelData

                        readonly property bool isSeparator: modelData.separator === true

                        height: isSeparator ? 7 : Theme.rowH
                        enabled: !isSeparator && modelData.enabled !== false
                        checkable: modelData.checkable === true
                        checked: modelData.checked === true
                        padding: 0

                        Accessible.name: isSeparator ? "" : modelData.text

                        onTriggered: root.triggered(modelData.action || "", checked)

                        background: Rectangle {
                            radius: Theme.rSm
                            color: mi.highlighted && !mi.isSeparator ? Theme.hoverLayer : "transparent"
                            Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durInstant) } }

                            Rectangle {
                                anchors.centerIn: parent
                                width: parent.width - Theme.s5 * 2
                                height: 1
                                color: Theme.divider
                                visible: mi.isSeparator
                            }
                        }

                        indicator: Item {}

                        contentItem: Item {
                            visible: !mi.isSeparator

                            Icon {
                                id: micon
                                name: mi.modelData.icon || ""
                                size: Theme.iconSm
                                color: mi.enabled ? Theme.textMuted : Theme.textDim
                                visible: name !== ""
                                anchors.left: parent.left
                                anchors.leftMargin: Theme.s5
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Icon {
                                name: "check"
                                size: Theme.iconSm
                                color: Theme.primary
                                visible: mi.checkable && mi.checked
                                anchors.left: parent.left
                                anchors.leftMargin: Theme.s5
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: mi.modelData.text || ""
                                color: mi.enabled ? Theme.text : Theme.textDim
                                font.family: Theme.uiFont
                                font.pixelSize: Theme.fsBody
                                elide: Text.ElideRight
                                anchors.left: parent.left
                                anchors.leftMargin: Theme.s5 + Theme.iconSm + Theme.s4
                                anchors.right: accel.left
                                anchors.rightMargin: Theme.s5
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                id: accel
                                text: mi.modelData.shortcut || ""
                                color: Theme.textDim
                                font.family: Theme.monoFont
                                font.pixelSize: Theme.fsTiny
                                anchors.right: parent.right
                                anchors.rightMargin: Theme.s5
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }
        }
    }

    property bool _anyOpen: false
}
