/***************************************************************************
//
//    Bible module: book → chapter → verse, plus full-text search and the
//    history of what has been sent live this session.
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root

    property bool searchOpen: App.bibleSearching
    property bool historyOpen: false

    function focusBooks() { bookFilter.forceFieldFocus() }
    function focusSearch() { root.historyOpen = false; searchField.forceFieldFocus() }

    Column {
        anchors.fill: parent
        spacing: 0

        // ── Toolbar ─────────────────────────────────────────────────────────
        Toolbar {
            width: parent.width

            Dropdown {
                width: 168
                anchors.verticalCenter: parent.verticalCenter
                iconName: "book-marked"
                label: qsTr("Bible version")
                model: App.bibleVersions
                currentIndex: App.bibleVersionIndex
                displayText: App.bibleVersions.length > 0 ? currentText
                                                          : qsTr("No Bible installed")
                enabled: App.bibleVersions.length > 0
                onActivated: App.bibleVersionIndex = currentIndex
            }

            ToolSep {}

            InputField {
                id: searchField
                width: 260
                anchors.verticalCenter: parent.verticalCenter
                role: "search"
                placeholder: qsTr("Search the Bible…")
                onAccepted: App.bibleSearch(text, scope.currentIndex, mode.currentIndex === 1)
                onEdited: if (text === "") App.bibleClearSearch()
            }

            Dropdown {
                id: scope
                width: 132
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Search scope")
                model: [qsTr("Whole Bible"), qsTr("This book"), qsTr("This chapter")]
            }

            Dropdown {
                id: mode
                width: 124
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Match mode")
                model: [qsTr("Phrase"), qsTr("Any word")]
            }

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "text-search"
                text: qsTr("Search")
                onClicked: App.bibleSearch(searchField.text, scope.currentIndex,
                                           mode.currentIndex === 1)
            }

            Item { width: Theme.s5; height: 1 }

            ToolBtn {
                anchors.verticalCenter: parent.verticalCenter
                iconName: "clock"
                text: qsTr("History")
                checkable: true
                checked: root.historyOpen
                onClicked: root.historyOpen = !root.historyOpen
            }
        }

        // ── Body ────────────────────────────────────────────────────────────
        SplitView {
            width: parent.width
            height: parent.height - Theme.toolbarH
            orientation: Qt.Horizontal
            handle: SplitGrip {}

            // Books
            Panel {
                SplitView.preferredWidth: 214
                SplitView.minimumWidth: 168
                title: qsTr("Books")
                iconName: "book-open"

                Column {
                    anchors.fill: parent
                    spacing: 0

                    Item {
                        width: parent.width
                        height: Theme.toolbarH

                        InputField {
                            id: bookFilter
                            anchors.fill: parent
                            anchors.margins: Theme.s4
                            role: "search"
                            placeholder: qsTr("Filter books")
                            onEdited: App.bibleBookFilter = text
                            onCleared: App.bibleBookFilter = ""
                        }
                    }

                    SelectList {
                        width: parent.width
                        height: parent.height - Theme.toolbarH
                        model: App.bibleBooks
                        currentIndex: App.bibleBookIndex
                        showIcons: false
                        emptyIcon: App.bibleVersions.length === 0 ? "database" : "book-open"
                        emptyTitle: App.bibleVersions.length === 0
                                    ? qsTr("No Bible installed")
                                    : qsTr("No matching book")
                        emptyHint: App.bibleVersions.length === 0
                                   ? qsTr("Download a Bible from Manage Data and it will show up here.")
                                   : qsTr("Clear the filter to see every book again.")
                        emptyAction: App.bibleVersions.length === 0 ? qsTr("Manage data") : ""
                        onEmptyActionTriggered: App.invoke("manageData")
                        onClicked: function(i) { App.bibleBookIndex = i }
                    }
                }
            }

            // Chapters
            Panel {
                SplitView.preferredWidth: 92
                SplitView.minimumWidth: 72
                SplitView.maximumWidth: 140
                title: qsTr("Ch.")
                iconName: "hash"

                GridView {
                    id: chapters
                    anchors.fill: parent
                    anchors.margins: Theme.s3
                    clip: true
                    cellWidth: 34
                    cellHeight: 30
                    model: App.bibleChapterCount
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: StyledScrollBar {}

                    delegate: Item {
                        required property int index
                        width: chapters.cellWidth
                        height: chapters.cellHeight

                        readonly property bool active: index === App.bibleChapterIndex

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            radius: Theme.rSm
                            color: parent.active ? Theme.primary
                                 : cma.containsMouse ? Theme.hoverLayer
                                 : "transparent"
                            border.width: parent.active ? 0 : 1
                            border.color: Theme.border

                            Behavior on color { ColorAnimation { duration: Theme.dur(Theme.durFast) } }

                            Text {
                                anchors.centerIn: parent
                                text: index + 1
                                color: parent.parent.active ? Theme.textOnAccent : Theme.textMuted
                                font.family: Theme.monoFont
                                font.pixelSize: Theme.fsSmall
                                font.weight: parent.parent.active ? Theme.wSemi : Theme.wRegular
                            }
                        }

                        MouseArea {
                            id: cma
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: App.bibleChapterIndex = index
                        }
                    }
                }
            }

            // Verses
            Panel {
                SplitView.fillWidth: true
                SplitView.minimumWidth: 280
                title: App.bibleBooks.length > 0 && App.bibleBookIndex >= 0
                       ? App.bibleBooks[App.bibleBookIndex].text + " " + (App.bibleChapterIndex + 1)
                       : qsTr("Chapter")
                iconName: "book-open"
                accent: Theme.primary

                headerItems: [
                    IconButton {
                        iconName: "chevron-left"
                        ToolTip.text: qsTr("Previous chapter")
                        onClicked: App.bibleStep(-1)
                    },
                    IconButton {
                        iconName: "chevron-right"
                        ToolTip.text: qsTr("Next chapter")
                        onClicked: App.bibleStep(1)
                    },
                    ToolSep {},
                    ToolBtn {
                        iconName: "list-plus"
                        text: qsTr("Schedule")
                        onClicked: App.bibleAddToSchedule()
                    },
                    Btn {
                        text: qsTr("Go Live")
                        iconName: "cast"
                        variant: "live"
                        enabled: App.bibleVerses.length > 0
                        onClicked: App.bibleGoLive(verses.selection.length > 0
                                                  ? verses.selection : [0])
                    }
                ]

                SelectList {
                    id: verses
                    anchors.fill: parent
                    model: App.bibleVerses
                    multiSelect: true
                    autoSelectFirst: true
                    wrapText: true
                    showIcons: false
                    rowHeight: Theme.rowHTall
                    emptyIcon: "book-open"
                    emptyTitle: App.bibleVersions.length === 0 ? qsTr("No Bible loaded")
                                                              : qsTr("Nothing loaded")
                    emptyHint: App.bibleVersions.length === 0
                               ? qsTr("Install a Bible module first — everything else in this panel needs one.")
                               : qsTr("Pick a book and chapter to read it here.")
                    onActivated: App.bibleGoLive(selection)
                }
            }

            // Search results / history
            Panel {
                id: aside
                SplitView.preferredWidth: 280
                SplitView.minimumWidth: 200
                visible: root.searchOpen || root.historyOpen
                title: root.historyOpen ? qsTr("History") : qsTr("Search results")
                iconName: root.historyOpen ? "clock" : "text-search"

                headerItems: [
                    IconButton {
                        iconName: "trash-2"
                        danger: true
                        visible: root.historyOpen
                        ToolTip.text: qsTr("Clear history")
                        onClicked: App.bibleClearHistory()
                    },
                    IconButton {
                        iconName: "x"
                        ToolTip.text: qsTr("Close")
                        onClicked: {
                            if (root.historyOpen) root.historyOpen = false
                            else { App.bibleClearSearch(); searchField.clear() }
                        }
                    }
                ]

                SelectList {
                    anchors.fill: parent
                    rowHeight: root.historyOpen ? Theme.rowH : 52
                    wrapText: !root.historyOpen
                    showIcons: root.historyOpen
                    model: root.historyOpen ? App.bibleHistory : App.bibleSearchResults
                    emptyIcon: root.historyOpen ? "clock" : "text-search"
                    emptyTitle: root.historyOpen ? qsTr("Nothing shown yet")
                                                 : qsTr("No results")
                    emptyHint: root.historyOpen
                               ? qsTr("Verses you send live appear here so you can go back to them.")
                               : qsTr("Try a shorter phrase, or widen the scope to the whole Bible.")
                    onClicked: function(i) {
                        if (root.historyOpen) App.bibleOpenHistory(i)
                        else App.bibleOpenSearchResult(i)
                    }
                }
            }
        }
    }
}
