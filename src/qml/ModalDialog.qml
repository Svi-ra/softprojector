/***************************************************************************
//
//    Modal shell for in-workspace prompts (confirm, rename, about).
//    Scales up from centre with a blurred-out scrim behind it; Escape always
//    closes, and destructive confirms colour their own button only.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Dialog {
    id: root

    property string iconName: ""
    property string subtitle: ""
    property string acceptText: qsTr("OK")
    property string rejectText: qsTr("Cancel")
    property bool destructive: false
    property bool showFooter: true
    property alias extraFooterItems: extraFooter.data

    default property alias dialogContent: bodyHolder.data

    anchors.centerIn: parent
    modal: true
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    implicitWidth: 420

    Overlay.modal: Rectangle {
        color: Theme.scrim
        Behavior on opacity { NumberAnimation { duration: Theme.dur(Theme.durBase) } }
    }

    background: Rectangle {
        color: Theme.raised
        radius: Theme.rXl
        border.width: 1
        border.color: Theme.borderStrong
    }

    header: Item {
        implicitHeight: root.title !== "" ? 56 : 0
        visible: root.title !== ""

        Row {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: Theme.s7
            anchors.rightMargin: Theme.s7
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.s5

            Rectangle {
                width: 32; height: 32; radius: Theme.rLg
                visible: root.iconName !== ""
                color: root.destructive ? Theme.dangerSoft : Theme.overlay
                anchors.verticalCenter: parent.verticalCenter

                Icon {
                    anchors.centerIn: parent
                    name: root.iconName
                    size: Theme.iconMd
                    color: root.destructive ? Theme.danger : Theme.primary
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 1

                Text {
                    text: root.title
                    color: Theme.text
                    font.family: Theme.uiFont
                    font.pixelSize: Theme.fsLarge
                    font.weight: Theme.wSemi
                }

                Text {
                    text: root.subtitle
                    visible: root.subtitle !== ""
                    color: Theme.textDim
                    font.family: Theme.uiFont
                    font.pixelSize: Theme.fsSmall
                }
            }
        }
    }

    contentItem: Item {
        id: bodyHolder
        implicitHeight: childrenRect.height
    }

    footer: Item {
        implicitHeight: root.showFooter ? 60 : Theme.s7
        visible: root.showFooter

        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: Theme.divider
        }

        Row {
            id: extraFooter
            anchors.left: parent.left
            anchors.leftMargin: Theme.s7
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.s4
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: Theme.s7
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.s4

            Btn {
                text: root.rejectText
                variant: "quiet"
                visible: root.rejectText !== ""
                onClicked: root.reject()
            }

            Btn {
                text: root.acceptText
                variant: root.destructive ? "danger" : "primary"
                visible: root.acceptText !== ""
                onClicked: root.accept()
            }
        }
    }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0; to: 1
                              duration: Theme.dur(Theme.durBase); easing.type: Theme.easeOut }
            NumberAnimation { property: "scale"; from: 0.94; to: 1
                              duration: Theme.dur(Theme.durBase); easing.type: Theme.easeEmph }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 1; to: 0
                              duration: Theme.dur(Theme.durFast); easing.type: Theme.easeIn }
            NumberAnimation { property: "scale"; from: 1; to: 0.97
                              duration: Theme.dur(Theme.durFast); easing.type: Theme.easeIn }
        }
    }
}
