/***************************************************************************
//
//    Themed combo box. The popup animates out of the field rather than
//    appearing, so the origin of the list is never ambiguous.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

ComboBox {
    id: root

    property string label: ""
    property string iconName: ""

    implicitHeight: Theme.ctrlH
    implicitWidth: 160
    padding: 0
    font.family: Theme.uiFont
    font.pixelSize: Theme.fsBody

    Accessible.name: label !== "" ? label : qsTr("Options")

    background: Rectangle {
        radius: Theme.rMd
        color: root.pressed ? Theme.pressedLayer
             : root.hovered ? Theme.hoverLayer
             : Theme.inset
        border.width: 1
        border.color: root.activeFocus || root.popup.visible ? Theme.primary : Theme.border

        Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durFast) } }
        Behavior on border.color { ColorAnimation { duration: Theme.dur(Theme.durFast) } }
    }

    contentItem: Row {
        leftPadding: Theme.s5
        spacing: Theme.s3

        Icon {
            name: root.iconName
            size: Theme.iconSm
            color: Theme.textDim
            visible: root.iconName !== ""
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: root.displayText
            color: root.enabled ? Theme.text : Theme.textDim
            font: root.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            anchors.verticalCenter: parent.verticalCenter
            width: root.width - Theme.s5 - Theme.hitMin
                   - (root.iconName !== "" ? Theme.iconSm + Theme.s3 : 0)
        }
    }

    indicator: Icon {
        name: "chevron-down"
        size: Theme.iconSm
        color: Theme.textDim
        x: root.width - width - Theme.s5
        y: (root.height - height) / 2
        rotation: root.popup.visible ? 180 : 0
        Behavior on rotation {
            NumberAnimation { duration: Theme.dur(Theme.durBase); easing.type: Theme.easeStd }
        }
    }

    delegate: ItemDelegate {
        width: root.width
        height: Theme.rowH
        padding: 0
        hoverEnabled: true

        contentItem: Text {
            text: root.textRole
                  ? (Array.isArray(root.model)
                     ? modelData[root.textRole]
                     : model[root.textRole])
                  : modelData
            color: root.currentIndex === index ? Theme.primary : Theme.text
            font.family: Theme.uiFont
            font.pixelSize: Theme.fsBody
            font.weight: root.currentIndex === index ? Theme.wMedium : Theme.wRegular
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            leftPadding: Theme.s5
            rightPadding: Theme.s5
            anchors.fill: parent
        }

        background: Rectangle {
            color: parent.hovered ? Theme.hoverLayer : "transparent"
            Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durInstant) } }
        }
    }

    popup: Popup {
        y: root.height + 4
        width: root.width
        implicitHeight: Math.min(contentItem.implicitHeight + 8, 300)
        padding: 4

        background: Rectangle {
            color: Theme.overlay
            radius: Theme.rLg
            border.width: 1
            border.color: Theme.borderStrong
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: StyledScrollBar {}
        }

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1
                              duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
            NumberAnimation { property: "scale"; from: 0.96; to: 1
                              duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
        }
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1; to: 0
                              duration: Theme.dur(Theme.durInstant); easing.type: Theme.easeIn }
        }
    }

    opacity: enabled ? 1.0 : Theme.disabledOpacity
}
