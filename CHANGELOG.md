# Changelog

## Unreleased — Qt Quick workspace

The operator interface was rebuilt in Qt Quick. The projection pipeline,
database access, themes and image generation are unchanged; only the window the
operator looks at was replaced.

### Added

**Design system** — `src/qml/Theme.qml`, a singleton holding every colour,
size, duration and easing curve in the interface. Five surface depths, three
text tiers that each clear 4.5:1 on their own surface in both light and dark,
semantic intent colours, state layers, a 4px spacing scale, and one motion
rhythm. Nothing else in the UI holds a hex value. `Theme.dur()` wraps every
animation duration so `reducedMotion` switches the whole app at once.

Light and dark modes. Dark is the default and the preference persists via
`QtCore.Settings`; toggle from the nav rail footer or `Ctrl+D`.

**Component library** — 23 reusable QML types in `src/qml/`:

| | |
| --- | --- |
| Content | `Icon` `Panel` `SelectList` `ListRow` `Badge` `EmptyState` |
| Controls | `IconButton` `ToolBtn` `Btn` `InputField` `Dropdown` `Toggle` `SliderBar` `Segmented` |
| Layout | `Toolbar` `ToolSep` `SplitGrip` `StyledScrollBar` |
| Shell | `NavRail` `AppMenuBar` `ModalDialog` `Transport` `Toaster` |

`SelectList` carries keyboard navigation, empty states, optional multi-select,
text wrapping and a card variant, so every list in the app behaves identically.

**Workspace** — `AppShell.qml`: title bar with menus and schedule state, a
navigation rail, and a `SplitView` of schedule · module · live. The two outer
columns never move, so the operator's hands always know where the running order
and Show are. Module panels cross-fade rather than cutting.

All five module panels rewritten: `BiblePanel` `SongsPanel` `PicturesPanel`
`MediaPanel` `AnnouncePanel`, plus `SchedulePanel` and `LivePanel`.

**`Workspace` view-model** — `src/headers/workspace.hpp`,
`src/sources/workspace.cpp`. Owns the browsing state of every module and
exposes it to QML as properties. It does not drive the projector: going live
emits a request signal that `SoftProjector` performs, then pushes the resulting
live state back through `setLive*()`.

**Image providers** — `LucideProvider` renders Lucide SVGs at the exact size
and colour requested, so icons stay crisp at any DPI from one file per glyph.
`ThumbProvider` serves slide thumbnails from database BLOBs to the pictures
grid.

**Lucide icon set** — the 96 icons the interface draws, in
`src/icons/lucide/` (ISC licence included).

**CMake build** — `src/CMakeLists.txt`, with the QML files as a proper
`qt_add_qml_module` (`SoftProjector.Ui`).

**Documentation** — build and interface guide in `README.md`; design system,
component reference and the C++ boundary in `src/qml/README.md`.

### Changed

- `SoftProjector` hosts a `QQuickWidget` instead of `softprojector.ui`. The
  schedule file format, printing, settings, display positioning and slide
  stepping are the same code; only their inputs moved from widget lookups to
  `Workspace` calls.
- Green now means exactly one thing — *on the projector right now*. It appears
  on Go Live, Show, the on-air badge and the status bar, nowhere else.
- Verse numbers render as tabular figures beside the text rather than being
  folded into it; chapters open with the first verse selected.
- Keyboard shortcuts are unchanged (`F6`–`F10`, `PgUp`/`PgDn`, `Shift+F5`,
  `Ctrl+N`/`O`/`S`/`P`) so no operator has to relearn a service.
- Application now sets an organisation name, required for `QSettings`, and uses
  the Fusion style so the remaining widget dialogs sit with the dark palette.

### Removed

- **qmake.** `src/softProjector.pro` is gone; CMake is the build system — Qt 6
  QML modules need it.
- The six replaced module widgets (`songwidget` `biblewidget` `announcewidget`
  `picturewidget` `mediawidget` `mediacontrol`), `videoplayerwidget`, their
  `.ui` forms, and `softprojector.ui` — 22 files.
- The vendored `QMediaPlaylist` backport in `3rdparty/`. It existed for two enum
  constants that `QMediaPlayer::Infinite` and `::Once` already provide.

### Notes for contributors

Three naming choices in the QML that look arbitrary and are not:

- The icon property is `iconName`, not `icon` — `AbstractButton.icon` is `FINAL`
  and cannot be shadowed.
- `ToolBtn` avoids colliding with `QtQuick.Controls.ToolButton`.
- `SplitGrip` avoids colliding with the `SplitHandle` attached type.

The remaining ~20 secondary dialogs (settings, manage data, song and
announcement editors, about) are still Qt Widgets. They inherit the dark
palette through Fusion but have not had a design pass.
