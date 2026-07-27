/***************************************************************************
//
//    Segmented control. The selection indicator slides between segments so
//    the change of state is one movement, not two blinks.
//
***************************************************************************/

import QtQuick

Rectangle {
    id: root

    property var options: []          // ["A", "B"] or [{ text, icon }]
    property int currentIndex: 0
    property bool compact: false

    signal activated(int index)

    function _label(o) { return (typeof o === "string") ? o : (o.text || "") }
    function _icon(o) { return (typeof o === "string") ? "" : (o.icon || "") }

    implicitHeight: Theme.ctrlH
    implicitWidth: row.implicitWidth + 6
    radius: Theme.rMd
    color: Theme.inset
    border.width: 1
    border.color: Theme.border

    Rectangle {
        id: marker
        y: 3
        height: parent.height - 6
        width: root.options.length > 0 ? (root.width - 6) / root.options.length : 0
        x: 3 + root.currentIndex * width
        radius: Theme.rSm
        color: Theme.raised
        border.width: 1
        border.color: Theme.borderStrong

        Behavior on x {
            NumberAnimation { duration: Theme.dur(Theme.durBase); easing.type: Theme.easeEmph }
        }
    }

    Row {
        id: row
        anchors.fill: parent
        anchors.margins: 3

        Repeater {
            model: root.options

            delegate: Item {
                id: seg
                required property var modelData
                required property int index

                width: root.options.length > 0 ? (root.width - 6) / root.options.length : 0
                height: parent.height

                readonly property bool active: root.currentIndex === index

                Row {
                    anchors.centerIn: parent
                    spacing: Theme.s3

                    Icon {
                        name: root._icon(seg.modelData)
                        size: Theme.iconSm
                        visible: name !== ""
                        color: seg.active ? Theme.text : Theme.textDim
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: root._label(seg.modelData)
                        visible: !root.compact && text !== ""
                        color: seg.active ? Theme.text : Theme.textDim
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fsSmall
                        font.weight: seg.active ? Theme.wSemi : Theme.wMedium
                        anchors.verticalCenter: parent.verticalCenter

                        Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durFast) } }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.currentIndex = seg.index
                        root.activated(seg.index)
                    }
                }

                Accessible.role: Accessible.RadioButton
                Accessible.name: root._label(modelData)
                Accessible.checked: active
            }
        }
    }
}
