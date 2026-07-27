/***************************************************************************
//
//    Single-line text input with an optional leading icon and clear button.
//    Set `role: "search"` for the magnifier + clear affordance.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

FocusScope {
    id: root

    property alias text: field.text
    property alias placeholder: field.placeholderText
    property alias validator: field.validator
    property alias inputMethodHints: field.inputMethodHints
    property alias readOnly: field.readOnly
    property alias echoMode: field.echoMode
    property alias horizontalAlignment: field.horizontalAlignment

    property string iconName: ""
    property string role: ""          // "search" adds magnifier + clear
    property bool clearable: role === "search"

    signal accepted()
    signal edited()
    signal cleared()

    function clear() { field.clear(); root.cleared() }
    function selectAllText() { field.selectAll() }
    function forceFieldFocus() { field.forceActiveFocus() }

    implicitHeight: Theme.ctrlH
    implicitWidth: 180

    Rectangle {
        anchors.fill: parent
        radius: Theme.rMd
        color: root.enabled ? Theme.inset : "transparent"
        border.width: 1
        border.color: field.activeFocus ? Theme.primary : Theme.border

        Behavior on border.color {
            ColorAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
        }

        // Focus ring sits outside the border so it never shifts the layout.
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2
            radius: parent.radius + 2
            color: "transparent"
            border.width: 2
            border.color: Theme.primary
            opacity: field.activeFocus ? 0.28 : 0
            Behavior on opacity { NumberAnimation { duration: Theme.dur(Theme.durFast) } }
        }

        Icon {
            id: lead
            name: root.role === "search" && root.iconName === "" ? "search" : root.iconName
            size: Theme.iconSm
            color: field.activeFocus ? Theme.primary : Theme.textDim
            visible: name !== ""
            anchors.left: parent.left
            anchors.leftMargin: Theme.s4
            anchors.verticalCenter: parent.verticalCenter
        }

        TextField {
            id: field
            anchors.fill: parent
            anchors.leftMargin: lead.visible ? Theme.s4 + lead.width + Theme.s3 : Theme.s5
            anchors.rightMargin: clearBtn.visible ? Theme.hitMin : Theme.s5

            focus: true
            selectByMouse: true
            color: Theme.text
            placeholderTextColor: Theme.textDim
            selectionColor: Theme.primary
            selectedTextColor: Theme.textOnAccent
            font.family: Theme.uiFont
            font.pixelSize: Theme.fsBody
            verticalAlignment: TextInput.AlignVCenter
            background: null
            padding: 0

            Accessible.name: placeholderText

            onAccepted: root.accepted()
            onTextEdited: root.edited()
            Keys.onEscapePressed: if (root.clearable && text !== "") root.clear()
        }

        IconButton {
            id: clearBtn
            iconName: "x"
            iconSize: Theme.iconSm
            ToolTip.text: qsTr("Clear")
            visible: root.clearable && field.text !== ""
            width: Theme.hitMin
            height: Theme.hitMin
            anchors.right: parent.right
            anchors.rightMargin: Theme.s1
            anchors.verticalCenter: parent.verticalCenter
            onClicked: { root.clear(); field.forceActiveFocus() }
        }
    }

    opacity: enabled ? 1.0 : Theme.disabledOpacity
}
