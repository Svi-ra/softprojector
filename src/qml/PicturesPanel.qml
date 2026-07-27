/***************************************************************************
//
//    Pictures module: slide shows on the left, a thumbnail grid on the right.
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

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "folder-plus"
                text: qsTr("New slide show")
                onClicked: App.invoke("slideshowNew")
            }

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "square-pen"
                text: qsTr("Edit")
                enabled: App.slideshowIndex >= 0
                onClicked: App.invoke("slideshowEdit")
            }

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "trash-2"
                text: qsTr("Delete")
                danger: true
                enabled: App.slideshowIndex >= 0
                onClicked: App.invoke("slideshowDelete")
            }

            ToolSep {}

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "list-plus"
                text: qsTr("Add to schedule")
                enabled: App.slideshowIndex >= 0
                onClicked: App.slideshowAddToSchedule()
            }
        }

        SplitView {
            width: parent.width
            height: parent.height - Theme.toolbarH
            orientation: Qt.Horizontal
            handle: SplitGrip {}

            Panel {
                SplitView.preferredWidth: 300
                SplitView.minimumWidth: 220
                title: qsTr("Slide shows")
                iconName: "images"

                SelectList {
                    anchors.fill: parent
                    model: App.slideshows
                    currentIndex: App.slideshowIndex
                    rowHeight: Theme.rowHTall
                    emptyIcon: "images"
                    emptyTitle: qsTr("No slide shows")
                    emptyHint: qsTr("Create one and drop your images into it.")
                    emptyAction: qsTr("New slide show")
                    onClicked: function(i) { App.slideshowIndex = i }
                    onEmptyActionTriggered: App.invoke("slideshowNew")
                }
            }

            Panel {
                SplitView.fillWidth: true
                SplitView.minimumWidth: 320
                title: App.slideshowName !== "" ? App.slideshowName : qsTr("Slides")
                iconName: "layout-grid"
                accent: Theme.primary

                headerItems: [
                    Btn {
                        text: qsTr("Go Live")
                        iconName: "cast"
                        variant: "live"
                        enabled: App.slides.length > 0
                        onClicked: App.slideshowGoLive(grid.currentIndex)
                    }
                ]

                GridView {
                    id: grid
                    anchors.fill: parent
                    anchors.margins: Theme.s5
                    clip: true
                    cellWidth: 152
                    cellHeight: 116
                    model: App.slides
                    currentIndex: App.slideIndex
                    boundsBehavior: Flickable.StopAtBounds
                    visible: count > 0
                    ScrollBar.vertical: StyledScrollBar {}

                    delegate: Item {
                        id: cell
                        required property var modelData
                        required property int index

                        width: grid.cellWidth
                        height: grid.cellHeight

                        readonly property bool active: index === App.slideIndex

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: Theme.s3
                            radius: Theme.rLg
                            color: cell.active ? Theme.selectedLayer
                                 : gma.containsMouse ? Theme.hoverLayer
                                 : Theme.inset
                            border.width: 1
                            border.color: cell.active ? Theme.primary : Theme.border

                            Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durFast) } }
                            Behavior on border.color { ColorAnimation { duration: Theme.dur(Theme.durFast) } }

                            Image {
                                anchors.fill: parent
                                anchors.margins: Theme.s3
                                anchors.bottomMargin: 18
                                source: cell.modelData.thumb || ""
                                fillMode: Image.PreserveAspectFit
                                asynchronous: true
                                smooth: true
                            }

                            Text {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.margins: Theme.s3
                                text: cell.index + 1
                                color: cell.active ? Theme.primary : Theme.textDim
                                font.family: Theme.monoFont
                                font.pixelSize: Theme.fsTiny
                                horizontalAlignment: Text.AlignHCenter
                            }

                            scale: gma.pressed ? 0.97 : 1.0
                            Behavior on scale { NumberAnimation { duration: Theme.dur(Theme.durInstant) } }
                        }

                        MouseArea {
                            id: gma
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: App.slideIndex = cell.index
                            onDoubleClicked: App.slideshowGoLive(cell.index)
                        }
                    }
                }

                EmptyState {
                    anchors.fill: parent
                    visible: grid.count === 0
                    iconName: "image-plus"
                    title: App.slideshowIndex < 0 ? qsTr("Nothing selected") : qsTr("No slides")
                    hint: App.slideshowIndex < 0
                          ? qsTr("Pick a slide show on the left.")
                          : qsTr("Open the editor to add images to this show.")
                    actionText: App.slideshowIndex >= 0 ? qsTr("Edit slide show") : ""
                    onActionTriggered: App.invoke("slideshowEdit")
                }
            }
        }
    }
}
