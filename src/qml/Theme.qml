/***************************************************************************
//
//    SoftProjector - an open source media projection software
//
//    Design tokens for the SoftProjector workspace.
//    Single source of truth for colour, type, spacing, radius and motion.
//    Nothing else in the UI is allowed to hardcode a colour.
//
***************************************************************************/

pragma Singleton

import QtQuick
import QtCore

QtObject {
    id: root

    // ── Mode ────────────────────────────────────────────────────────────────
    property bool dark: true

    property Settings _store: Settings {
        category: "workspace"
        property alias dark: root.dark
    }

    // ── Surfaces ────────────────────────────────────────────────────────────
    // Five depth levels. Never pure black: OLED smear + harsh contrast.
    readonly property color bg:        dark ? "#0B1120" : "#F1F5F9"
    readonly property color surface:   dark ? "#0F172A" : "#FFFFFF"
    readonly property color raised:    dark ? "#16203A" : "#FFFFFF"
    readonly property color overlay:   dark ? "#1E293B" : "#F8FAFC"
    readonly property color inset:     dark ? "#080D18" : "#E9EEF5"

    // ── Lines ───────────────────────────────────────────────────────────────
    readonly property color border:       dark ? "#1F2A44" : "#E2E8F0"
    readonly property color borderStrong: dark ? "#334155" : "#CBD5E1"
    readonly property color divider:      dark ? "#172033" : "#EDF1F6"

    // ── Text ────────────────────────────────────────────────────────────────
    readonly property color text:      dark ? "#F1F5F9" : "#0F172A"
    readonly property color textMuted: dark ? "#94A3B8" : "#475569"
    // Both dim values clear 4.5:1 on their own surface — this tier still
    // carries real information (verse numbers, songbook names, counts).
    readonly property color textDim:   dark ? "#7C8CA8" : "#64748B"
    readonly property color textOnAccent: "#FFFFFF"

    // ── Intent ──────────────────────────────────────────────────────────────
    // primary = selection / focus / interactive
    // live    = "on the projector right now". Green reads as GO in every venue.
    readonly property color primary:     dark ? "#6366F1" : "#4F46E5"
    readonly property color primarySoft: dark ? "#312E81" : "#E0E7FF"
    readonly property color live:        dark ? "#22C55E" : "#16A34A"
    readonly property color liveSoft:    dark ? "#14361F" : "#DCFCE7"
    readonly property color danger:      dark ? "#EF4444" : "#DC2626"
    readonly property color dangerSoft:  dark ? "#3B1517" : "#FEE2E2"
    readonly property color warning:     dark ? "#F59E0B" : "#D97706"
    readonly property color info:        dark ? "#38BDF8" : "#0284C7"

    // ── State layers ────────────────────────────────────────────────────────
    // Applied over a surface, never as a replacement colour.
    readonly property color hoverLayer:    dark ? Qt.rgba(1, 1, 1, 0.05) : Qt.rgba(0, 0, 0, 0.04)
    readonly property color pressedLayer:  dark ? Qt.rgba(1, 1, 1, 0.09) : Qt.rgba(0, 0, 0, 0.08)
    readonly property color selectedLayer: dark ? Qt.rgba(99, 102, 241, 0.18) : Qt.rgba(79, 70, 229, 0.12)
    readonly property color scrim:         Qt.rgba(0, 0, 0, dark ? 0.62 : 0.42)

    readonly property real disabledOpacity: 0.38

    // ── Typography ──────────────────────────────────────────────────────────
    // Inter when the user has it; otherwise the platform UI face, which on
    // Windows 11 is Segoe UI Variable and is genuinely good. Resolved once
    // against the installed fonts rather than relying on substitution, so
    // every label in the app is measurably the same face.
    function _pick(wanted) {
        var have = Qt.fontFamilies()
        for (var i = 0; i < wanted.length; ++i)
            if (have.indexOf(wanted[i]) !== -1)
                return wanted[i]
        return Qt.application.font.family
    }

    readonly property string uiFont: _pick(["Inter", "Segoe UI Variable Text",
                                            "Segoe UI", "Noto Sans"])
    readonly property string monoFont: _pick(["JetBrains Mono", "Cascadia Mono",
                                              "Consolas", "DejaVu Sans Mono"])

    readonly property int fsMicro:  10
    readonly property int fsTiny:   11
    readonly property int fsSmall:  12
    readonly property int fsBody:   13
    readonly property int fsMedium: 14
    readonly property int fsLarge:  16
    readonly property int fsTitle:  19
    readonly property int fsHero:   24

    readonly property int wRegular: Font.Normal
    readonly property int wMedium:  Font.Medium
    readonly property int wSemi:    Font.DemiBold
    readonly property int wBold:    Font.Bold

    // Uppercase section labels need air or they read as noise.
    readonly property real trackingLabel: 0.9

    // ── Spacing (dense / dashboard rhythm, 4px base) ─────────────────────────
    readonly property int s1: 2
    readonly property int s2: 4
    readonly property int s3: 6
    readonly property int s4: 8
    readonly property int s5: 12
    readonly property int s6: 16
    readonly property int s7: 20
    readonly property int s8: 24
    readonly property int s9: 32

    // ── Radius ──────────────────────────────────────────────────────────────
    readonly property int rSm:   4
    readonly property int rMd:   6
    readonly property int rLg:   8
    readonly property int rXl:   12
    readonly property int rFull: 999

    // ── Metrics ─────────────────────────────────────────────────────────────
    readonly property int titleBarH:  38
    readonly property int navRailW:   60
    readonly property int toolbarH:   38
    readonly property int headerH:    34
    readonly property int statusBarH: 26
    readonly property int rowH:       30
    readonly property int rowHTall:   38
    readonly property int ctrlH:      28
    readonly property int iconSm:     14
    readonly property int iconMd:     16
    readonly property int iconLg:     20
    readonly property int iconXl:     24
    readonly property int hitMin:     28

    // ── Motion ──────────────────────────────────────────────────────────────
    // One rhythm everywhere. Exit is faster than enter.
    readonly property int durInstant: 90
    readonly property int durFast:    140
    readonly property int durBase:    200
    readonly property int durSlow:    280

    readonly property int easeOut:   Easing.OutCubic     // entering
    readonly property int easeIn:    Easing.InCubic      // exiting
    readonly property int easeStd:   Easing.InOutCubic   // moving within view
    readonly property int easeEmph:  Easing.OutBack      // attention

    // Respect the OS "reduce motion" preference where the platform reports it.
    property bool reducedMotion: false
    function dur(ms) { return reducedMotion ? 0 : ms }

    function toggleMode() { dark = !dark }

    // Colour of an icon/text drawn on a live (green) background.
    function on(bg) {
        return Qt.colorEqual(bg, root.live) || Qt.colorEqual(bg, root.primary)
                ? root.textOnAccent : root.text
    }
}
