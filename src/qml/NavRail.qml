/***************************************************************************
//
//    Primary navigation: the vertical rail of workspace modules.
//    Icon plus label — an icon-only rail is a memory test.
//    The active indicator slides between items so you can see where you came
//    from.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root

    // [{ key, icon, label, shortcut }]
    property var modules: []
    property string current: ""
    property alias footerItems: footer.data

    signal selected(string key)

    implicitWidth: Theme.navRailW
    color: Theme.bg

    Rectangle {
        anchors.right: parent.right
        width: 1
        height: parent.height
        color: Theme.border
    }

    Column {
        id: list
        anchors.top: parent.top
        anchors.topMargin: Theme.s4
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Theme.s1

        Repeater {
            model: root.modules

            delegate: Item {
                id: cell
                width: Theme.navRailW
                height: 52

                required property var modelData
                readonly property bool active: root.current === modelData.key

                Rectangle {
                    id: cellBg
                    anchors.fill: parent
                    anchors.leftMargin: Theme.s3
                    anchors.rightMargin: Theme.s3
                    anchors.topMargin: Theme.s1
                    anchors.bottomMargin: Theme.s1
                    radius: Theme.rLg
                    color: cell.active ? Theme.selectedLayer
                         : ma.pressed ? Theme.pressedLayer
                         : ma.containsMouse ? Theme.hoverLayer
                         : "transparent"

                    Behavior on color {
                        ColorAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
                    }
                }

                Column {
                    anchors.centerIn: parent
                    spacing: 3

                    Icon {
                        name: cell.modelData.icon
                        size: Theme.iconLg
                        color: cell.active ? Theme.primary
                             : ma.containsMouse ? Theme.text
                             : Theme.textDim
                        anchors.horizontalCenter: parent.horizontalCenter
                        scale: ma.pressed ? 0.9 : 1.0
                        Behavior on scale {
                            NumberAnimation { duration: Theme.dur(Theme.durInstant) }
                        }
                    }

                    Text {
                        text: cell.modelData.label
                        color: cell.active ? Theme.text : Theme.textDim
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fsMicro
                        font.weight: cell.active ? Theme.wSemi : Theme.wMedium
                        anchors.horizontalCenter: parent.horizontalCenter

                        Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durFast) } }
                    }
                }

                MouseArea {
                    id: ma
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.selected(cell.modelData.key)

                    ToolTip.visible: containsMouse && cell.modelData.shortcut !== undefined
                    ToolTip.delay: 700
                    ToolTip.text: cell.modelData.label + "   " + (cell.modelData.shortcut || "")
                }

                Accessible.role: Accessible.PageTab
                Accessible.name: modelData.label
                Accessible.selected: active
            }
        }
    }

    // Active indicator. One shared element rather than one per item, so it
    // travels between modules instead of blinking.
    Rectangle {
        id: marker
        width: 3
        height: 26
        radius: 2
        color: Theme.primary
        x: 0
        opacity: root.current !== "" ? 1 : 0

        y: {
            var i = 0
            for (var n = 0; n < root.modules.length; ++n)
                if (root.modules[n].key === root.current) { i = n; break }
            return list.y + i * (52 + Theme.s1) + (52 - height) / 2
        }

        Behavior on y {
            NumberAnimation { duration: Theme.dur(Theme.durBase); easing.type: Theme.easeEmph }
        }
        Behavior on opacity { NumberAnimation { duration: Theme.dur(Theme.durFast) } }
    }

    Column {
        id: footer
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.s4
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Theme.s2
    }
}
