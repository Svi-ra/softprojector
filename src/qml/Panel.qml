/***************************************************************************
//
//    The workspace unit: a bordered surface with an optional header strip.
//    Every region of the app — schedule, module, live — is a Panel, which is
//    what makes the layout read as one system.
//
//    Default content goes into `body`.
//
***************************************************************************/

import QtQuick

Rectangle {
    id: root

    default property alias content: body.data
    property alias body: body

    property string title: ""
    property string iconName: ""
    property color accent: Theme.textDim

    // Anything placed here sits on the right of the header strip.
    property alias headerItems: headerRow.data
    property bool showHeader: title !== "" || headerRow.children.length > 0

    color: Theme.surface
    radius: Theme.rLg
    border.width: 1
    border.color: Theme.border
    clip: true

    Column {
        anchors.fill: parent
        spacing: 0

        // ── Header ──────────────────────────────────────────────────────────
        Item {
            width: parent.width
            height: root.showHeader ? Theme.headerH : 0
            visible: root.showHeader

            Row {
                anchors.left: parent.left
                anchors.leftMargin: Theme.s5
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.s4

                Icon {
                    name: root.iconName
                    size: Theme.iconSm
                    color: root.accent
                    visible: root.iconName !== ""
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: root.title.toUpperCase()
                    color: Theme.textMuted
                    font.family: Theme.uiFont
                    font.pixelSize: Theme.fsTiny
                    font.weight: Theme.wSemi
                    font.letterSpacing: Theme.trackingLabel
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Row {
                id: headerRow
                anchors.right: parent.right
                anchors.rightMargin: Theme.s3
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.s1
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: Theme.divider
            }
        }

        // ── Body ────────────────────────────────────────────────────────────
        Item {
            id: body
            width: parent.width
            height: parent.height - (root.showHeader ? Theme.headerH : 0)
            clip: true
        }
    }
}
