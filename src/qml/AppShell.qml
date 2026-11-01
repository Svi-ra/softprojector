/***************************************************************************
//
//    SoftProjector workspace.
//
//    Four regions, left to right: navigation rail, schedule, the active
//    module, and the live column. The two outer columns never move, so the
//    operator's hands always know where Show and the running order are.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Item {
    id: shell

    anchors.fill: parent
    focus: true

    readonly property var modules: [
        { key: "bible",    icon: "book-open",    label: qsTr("Bible"),   shortcut: "F6" },
        { key: "songs",    icon: "music-4",      label: qsTr("Songs"),   shortcut: "F7" },
        { key: "pictures", icon: "images",       label: qsTr("Images"),  shortcut: "F9" },
        { key: "media",    icon: "clapperboard", label: qsTr("Media"),   shortcut: "F10" },
        { key: "announce", icon: "megaphone",    label: qsTr("Notices"), shortcut: "F8" }
    ]

    Rectangle {
        anchors.fill: parent
        color: Theme.bg

        Behavior on color {
            ColorAnimation { duration: Theme.dur(Theme.durSlow); easing.type: Theme.easeStd }
        }
    }

    // ── Title bar ───────────────────────────────────────────────────────────
    Rectangle {
        id: titleBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Theme.titleBarH
        color: Theme.surface

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Theme.border
        }

        Row {
            anchors.left: parent.left
            anchors.leftMargin: Theme.s5
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.s5

            Row {
                spacing: Theme.s4
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    width: 20; height: 20; radius: Theme.rSm
                    color: Theme.primary
                    anchors.verticalCenter: parent.verticalCenter

                    Icon {
                        anchors.centerIn: parent
                        name: "projector"
                        size: Theme.iconSm
                        color: Theme.textOnAccent
                    }
                }

                Text {
                    text: "SoftProjector"
                    color: Theme.text
                    font.family: Theme.uiFont
                    font.pixelSize: Theme.fsBody
                    font.weight: Theme.wSemi
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            AppMenuBar {
                anchors.verticalCenter: parent.verticalCenter
                menus: shell.menuModel
                onTriggered: function(action, checked) { App.invoke(action) }
            }
        }

        // Schedule name, centred, so the operator can see which service file
        // is open without hunting for it.
        Row {
            anchors.centerIn: parent
            spacing: Theme.s3
            visible: App.scheduleName !== ""

            Icon {
                name: "calendar-days"
                size: Theme.iconSm
                color: Theme.textDim
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: App.scheduleName + (App.scheduleDirty ? " •" : "")
                color: Theme.textMuted
                font.family: Theme.uiFont
                font.pixelSize: Theme.fsSmall
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: Theme.s4
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.s1

            ToolBtn {
                iconName: "file-plus"
                showLabel: false
                ToolTip.text: qsTr("New schedule")
                anchors.verticalCenter: parent.verticalCenter
                onClicked: App.invoke("newSchedule")
            }
            ToolBtn {
                iconName: "folder-open"
                showLabel: false
                ToolTip.text: qsTr("Open schedule")
                anchors.verticalCenter: parent.verticalCenter
                onClicked: App.invoke("openSchedule")
            }
            ToolBtn {
                iconName: "save"
                showLabel: false
                ToolTip.text: qsTr("Save schedule")
                anchors.verticalCenter: parent.verticalCenter
                onClicked: App.invoke("saveSchedule")
            }

            ToolSep {}

            ToolBtn {
                iconName: App.displayOn ? "monitor" : "monitor-x"
                text: App.displayOn ? qsTr("Display on") : qsTr("Display off")
                accent: App.displayOn
                ToolTip.text: qsTr("Turn the projector output on or off")
                anchors.verticalCenter: parent.verticalCenter
                onClicked: App.invoke("toggleDisplay")
            }
        }
    }

    // ── Navigation rail ─────────────────────────────────────────────────────
    NavRail {
        id: rail
        anchors.top: titleBar.bottom
        anchors.bottom: statusBar.top
        anchors.left: parent.left
        modules: shell.modules
        current: App.module
        onSelected: function(key) { App.module = key }

        footerItems: [
            IconButton {
                iconName: "database"
                ToolTip.text: qsTr("Manage data")
                onClicked: App.invoke("manageData")
            },
            IconButton {
                iconName: "settings"
                ToolTip.text: qsTr("Settings")
                onClicked: App.invoke("settings")
            },
            IconButton {
                iconName: Theme.dark ? "sun" : "moon"
                ToolTip.text: Theme.dark ? qsTr("Switch to light theme")
                                         : qsTr("Switch to dark theme")
                onClicked: Theme.toggleMode()
            }
        ]
    }

    // ── Workspace ───────────────────────────────────────────────────────────
    SplitView {
        id: split
        anchors.top: titleBar.bottom
        anchors.bottom: statusBar.top
        anchors.left: rail.right
        anchors.right: parent.right
        anchors.margins: Theme.s4
        orientation: Qt.Horizontal
        handle: SplitGrip {}

        SchedulePanel {
            SplitView.preferredWidth: 264
            SplitView.minimumWidth: 200
            SplitView.maximumWidth: 420
        }

        // Module stack. Panels cross-fade rather than cutting, so switching
        // modules reads as one workspace changing tools.
        Item {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 420

            Repeater {
                model: shell.modules

                delegate: Loader {
                    required property var modelData
                    anchors.fill: parent
                    active: App.module === modelData.key || item !== null
                    visible: opacity > 0
                    opacity: App.module === modelData.key ? 1 : 0
                    z: App.module === modelData.key ? 1 : 0

                    source: {
                        switch (modelData.key) {
                        case "bible":    return "BiblePanel.qml"
                        case "songs":    return "SongsPanel.qml"
                        case "pictures": return "PicturesPanel.qml"
                        case "media":    return "MediaPanel.qml"
                        case "announce": return "AnnouncePanel.qml"
                        }
                        return ""
                    }

                    Behavior on opacity {
                        NumberAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
                    }
                }
            }
        }

        LivePanel {
            id: livePanel
            SplitView.preferredWidth: 340
            SplitView.minimumWidth: 280
            SplitView.maximumWidth: 560
        }
    }

    // ── Status bar ──────────────────────────────────────────────────────────
    Rectangle {
        id: statusBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: Theme.statusBarH
        color: Theme.surface

        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: Theme.border
        }

        Row {
            anchors.left: parent.left
            anchors.leftMargin: Theme.s5
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.s5

            Row {
                spacing: Theme.s3
                anchors.verticalCenter: parent.verticalCenter

                Icon {
                    name: App.live ? "radio" : "circle-dot"
                    size: Theme.iconSm
                    color: App.live ? Theme.live : Theme.textDim
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: App.live ? qsTr("Projecting") : qsTr("Standing by")
                    color: App.live ? Theme.live : Theme.textDim
                    font.family: Theme.uiFont
                    font.pixelSize: Theme.fsTiny
                    font.weight: Theme.wMedium
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Text {
                text: "·"
                color: Theme.textDim
                font.pixelSize: Theme.fsTiny
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: App.screenCount > 1
                      ? qsTr("%n screen(s)", "", App.screenCount)
                      : qsTr("Single screen")
                color: Theme.textDim
                font.family: Theme.uiFont
                font.pixelSize: Theme.fsTiny
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: App.statusText
                visible: text !== ""
                color: Theme.textMuted
                font.family: Theme.uiFont
                font.pixelSize: Theme.fsTiny
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: Theme.s5
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.s5

            Text {
                text: qsTr("PgUp / PgDn to step slides")
                color: Theme.textDim
                font.family: Theme.uiFont
                font.pixelSize: Theme.fsMicro
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: "v" + App.versionString
                color: Theme.textDim
                font.family: Theme.monoFont
                font.pixelSize: Theme.fsMicro
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    // ── Toasts ──────────────────────────────────────────────────────────────
    Toaster { id: toaster; anchors.fill: parent }

    Connections {
        target: App
        function onToast(message, tone) { toaster.show(message, tone) }
    }

    // ── Menus ───────────────────────────────────────────────────────────────
    readonly property var menuModel: [
        {
            title: qsTr("File"),
            items: [
                { text: qsTr("New schedule"),   icon: "file-plus",   shortcut: "Ctrl+N", action: "newSchedule" },
                { text: qsTr("Open schedule…"), icon: "folder-open", shortcut: "Ctrl+O", action: "openSchedule" },
                { text: qsTr("Save schedule"),  icon: "save",        shortcut: "Ctrl+S", action: "saveSchedule" },
                { text: qsTr("Save schedule as…"), icon: "save",     action: "saveScheduleAs" },
                { text: qsTr("Close schedule"), action: "closeSchedule" },
                { separator: true },
                { text: qsTr("Print…"),         icon: "printer",     shortcut: "Ctrl+P", action: "print" },
                { text: qsTr("Print schedule…"), icon: "printer",    action: "printSchedule" },
                { separator: true },
                { text: qsTr("Exit"),           action: "quit" }
            ]
        },
        {
            title: qsTr("Edit"),
            items: [
                { text: qsTr("New"),      icon: "plus",      action: "editNew" },
                { text: qsTr("Edit"),     icon: "square-pen", action: "editEdit" },
                { text: qsTr("Copy"),     icon: "copy",      action: "editCopy" },
                { text: qsTr("Delete"),   icon: "trash-2",   action: "editDelete" },
                { separator: true },
                { text: qsTr("Manage data…"), icon: "database", action: "manageData" },
                { text: qsTr("Song counter…"), icon: "hash",  action: "songCounter" },
                { separator: true },
                { text: qsTr("Settings…"), icon: "settings",  action: "settings" }
            ]
        },
        {
            title: qsTr("Display"),
            items: [
                { text: qsTr("Show"),  icon: "cast",    shortcut: "Shift+F5", action: "show" },
                { text: qsTr("Hide"),  icon: "eye-off", action: "hide" },
                { text: qsTr("Clear"), icon: "eraser",  action: "clear" },
                { separator: true },
                { text: qsTr("Next slide"),     icon: "chevron-right", shortcut: "PgDn", action: "nextSlide" },
                { text: qsTr("Previous slide"), icon: "chevron-left",  shortcut: "PgUp", action: "prevSlide" },
                { separator: true },
                { text: qsTr("Turn display on/off"), icon: "monitor", action: "toggleDisplay" }
            ]
        },
        {
            title: qsTr("Help"),
            items: [
                { text: qsTr("Help…"),  icon: "circle-question-mark", shortcut: "F1", action: "help" },
                { text: qsTr("Donate…"), icon: "heart", action: "donate" },
                { separator: true },
                { text: qsTr("About SoftProjector"), icon: "info", action: "about" }
            ]
        }
    ]

    // ── Keyboard ────────────────────────────────────────────────────────────
    // These mirror the shortcuts the existing application already taught its
    // operators; changing them mid-service would be cruel.
    Shortcut { sequence: "F6";  onActivated: App.module = "bible" }
    Shortcut { sequence: "F7";  onActivated: App.module = "songs" }
    Shortcut { sequence: "F8";  onActivated: App.module = "announce" }
    Shortcut { sequence: "F9";  onActivated: App.module = "pictures" }
    Shortcut { sequence: "F10"; onActivated: App.module = "media" }
    Shortcut { sequence: "PgDown"; onActivated: App.invoke("nextSlide") }
    Shortcut { sequence: "PgUp";   onActivated: App.invoke("prevSlide") }
    Shortcut { sequence: "Shift+F5"; onActivated: App.invoke("show") }
    Shortcut { sequence: "Esc";       onActivated: App.invoke("hide") }
    Shortcut { sequence: "Shift+Esc"; onActivated: App.invoke("clear") }
    Shortcut { sequence: "Ctrl+Shift+B"; onActivated: App.invoke("show") }
    Shortcut { sequence: StandardKey.New;   onActivated: App.invoke("newSchedule") }
    Shortcut { sequence: StandardKey.Open;  onActivated: App.invoke("openSchedule") }
    Shortcut { sequence: StandardKey.Save;  onActivated: App.invoke("saveSchedule") }
    Shortcut { sequence: StandardKey.Print; onActivated: App.invoke("print") }
    Shortcut { sequence: StandardKey.HelpContents; onActivated: App.invoke("help") }
    Shortcut { sequence: "Ctrl+F"; onActivated: App.invoke("focusSearch") }
    Shortcut { sequence: "Ctrl+D"; onActivated: Theme.toggleMode() }
}
