/***************************************************************************
//
//    List of ListRows with keyboard navigation, an empty state and optional
//    multi-select. Every browsable list in the workspace is one of these,
//    which is why they all behave the same.
//
//    Model rows are maps: { text, secondary, trailing, icon, thumb }
//
***************************************************************************/

import QtQuick
import QtQuick.Controls.Basic

FocusScope {
    id: root

    property alias model: view.model
    property int currentIndex: -1
    property bool multiSelect: false
    property var selection: []          // extra selected rows when multiSelect
    property bool showIcons: true
    property bool wrapText: false
    property bool autoSelectFirst: false
    property bool cards: false
    property int rowHeight: Theme.rowH
    property string emptyIcon: "circle-dot"
    property string emptyTitle: ""
    property string emptyHint: ""
    property string emptyAction: ""
    property bool liveMarker: false     // colour selection green instead of indigo
    property int liveRow: -1

    signal activated(int index)         // double click / Enter
    signal clicked(int index)
    signal emptyActionTriggered()

    function positionAt(i) { view.positionViewAtIndex(i, ListView.Contain) }

    function _isSelected(i) {
        if (!multiSelect) return i === root.currentIndex
        return i === root.currentIndex || root.selection.indexOf(i) !== -1
    }

    function _select(i, modifiers) {
        if (root.multiSelect && (modifiers & Qt.ShiftModifier) && root.currentIndex >= 0) {
            var lo = Math.min(root.currentIndex, i)
            var hi = Math.max(root.currentIndex, i)
            var s = []
            for (var n = lo; n <= hi; ++n) s.push(n)
            root.selection = s
        } else if (root.multiSelect && (modifiers & Qt.ControlModifier)) {
            var c = root.selection.slice()
            var at = c.indexOf(i)
            if (at === -1) c.push(i); else c.splice(at, 1)
            root.selection = c
        } else {
            root.selection = [i]
            root.currentIndex = i
        }
        root.clicked(i)
    }

    // Lists that always need something showing (a chapter, a song's stanzas)
    // land on the first row rather than opening with a disabled Go Live.
    onModelChanged: if (autoSelectFirst) { currentIndex = 0; selection = [0] }

    ListView {
        id: view
        anchors.fill: parent
        anchors.margins: Theme.s2
        clip: true
        focus: true
        spacing: root.cards ? Theme.s3 : 1
        currentIndex: root.currentIndex
        boundsBehavior: Flickable.StopAtBounds
        highlightMoveDuration: Theme.dur(Theme.durFast)
        cacheBuffer: 400

        ScrollBar.vertical: StyledScrollBar {}

        // Entering rows stagger in. Capped so a 1000-verse chapter does not
        // spend three seconds assembling itself.
        add: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1
                              duration: Theme.dur(Theme.durFast) }
        }

        delegate: ListRow {
            required property var modelData
            required property int index

            width: view.width - (view.ScrollBar.vertical.visible ? Theme.s6 : 0)
            height: root.rowHeight

            text: modelData.text !== undefined ? modelData.text : modelData
            secondaryText: modelData.secondary || ""
            leadingText: modelData.number || ""
            wrap: root.wrapText
            card: root.cards
            trailingText: modelData.trailing || ""
            iconName: root.showIcons ? (modelData.icon || "") : ""
            selected: root._isSelected(index)
            isLive: root.liveMarker && index === root.liveRow

            // ItemDelegate's clicked() does not carry the modifier keys, and
            // shift/ctrl range selection is required for multi-verse. Own the
            // mouse here and feed hover back into the row.
            hoverEnabled: false
            hovering: ma.containsMouse

            MouseArea {
                id: ma
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton
                cursorShape: Qt.PointingHandCursor

                onClicked: function(mouse) {
                    root._select(index, mouse.modifiers)
                    root.forceActiveFocus()
                }
                onDoubleClicked: root.activated(index)
            }
        }

        Keys.onPressed: function(e) {
            if (e.key === Qt.Key_Return || e.key === Qt.Key_Enter) {
                if (root.currentIndex >= 0) root.activated(root.currentIndex)
                e.accepted = true
            } else if (e.key === Qt.Key_Down && root.currentIndex < count - 1) {
                root._select(root.currentIndex + 1, e.modifiers)
                view.positionViewAtIndex(root.currentIndex, ListView.Contain)
                e.accepted = true
            } else if (e.key === Qt.Key_Up && root.currentIndex > 0) {
                root._select(root.currentIndex - 1, e.modifiers)
                view.positionViewAtIndex(root.currentIndex, ListView.Contain)
                e.accepted = true
            } else if (e.key === Qt.Key_Home && count > 0) {
                root._select(0, 0); e.accepted = true
            } else if (e.key === Qt.Key_End && count > 0) {
                root._select(count - 1, 0); e.accepted = true
            }
        }
    }

    EmptyState {
        anchors.fill: parent
        visible: view.count === 0 && root.emptyTitle !== ""
        iconName: root.emptyIcon
        title: root.emptyTitle
        hint: root.emptyHint
        actionText: root.emptyAction
        onActionTriggered: root.emptyActionTriggered()
    }
}
