# SoftProjector workspace — design system

Everything here is one QML module, `SoftProjector.Ui`. Types are flat: any file
in this directory is usable from any other with no import.

## The one rule

**No component hardcodes a colour, size, duration or easing curve.** They all
come from `Theme.qml`, which is a singleton. If you find yourself typing a hex
value or a magic number, add a token instead.

## Theme.qml

| Group | Tokens |
| --- | --- |
| Surfaces | `bg` `surface` `raised` `overlay` `inset` — five depth levels, never pure black |
| Lines | `border` `borderStrong` `divider` |
| Text | `text` `textMuted` `textDim` `textOnAccent` — all three tiers clear 4.5:1 on their own surface in **both** modes |
| Intent | `primary` (selection/focus) `live` (on the projector) `danger` `warning` `info`, each with a `…Soft` background pair |
| State | `hoverLayer` `pressedLayer` `selectedLayer` `scrim` — layers over a surface, never a replacement colour |
| Type | `uiFont` `monoFont` (resolved once against installed fonts), `fs*` sizes, `w*` weights |
| Space | `s1`…`s9` on a 4px base, tuned dense for a dashboard |
| Radius | `rSm` `rMd` `rLg` `rXl` `rFull` |
| Metrics | `navRailW` `toolbarH` `rowH` `ctrlH` `icon*` … |
| Motion | `durInstant` `durFast` `durBase` `durSlow`, `easeOut` (enter) `easeIn` (exit) `easeStd` `easeEmph` |

`Theme.dark` drives the whole palette and is persisted via `QtCore.Settings`.
`Theme.dur(ms)` returns 0 when `reducedMotion` is set, so wrapping every
animation duration in it makes the whole app respect that preference at once.

Green is reserved for **live**. It appears on the Go Live buttons, the Show
button, the on-air badge and the status bar — nowhere else. That is what makes
"is something on the projector right now?" answerable from across a room.

## Components

| Component | Notes |
| --- | --- |
| `Icon` | A Lucide glyph. `name` is the icon's file name; it is rendered from SVG at the exact size and colour through the `lucide` image provider, so it stays crisp at any DPI |
| `IconButton` `ToolBtn` `Btn` | Icon-only, toolbar, and text buttons. `Btn.variant` is `primary` / `live` / `ghost` / `quiet` / `danger` |
| `Panel` | The workspace unit: bordered surface with an optional header strip. Every region of the app is one, which is what makes the layout read as a system |
| `SelectList` + `ListRow` | Every browsable list. Keyboard navigation, empty state, optional multi-select and a `cards` variant for slide-shaped content |
| `InputField` `Dropdown` `Toggle` `SliderBar` `Segmented` | Themed form controls |
| `Toolbar` `ToolSep` | Toolbar strip and group separator |
| `Badge` `EmptyState` `Toaster` | Status chip, "nothing here yet, do this next", transient messages |
| `NavRail` `AppMenuBar` `SplitGrip` `StyledScrollBar` `ModalDialog` `Transport` | Shell furniture |

Two names are deliberate: `ToolBtn` and `SplitGrip` avoid colliding with
`QtQuick.Controls.ToolButton` and the `SplitHandle` attached type. The custom
property is `iconName`, not `icon`, because `AbstractButton.icon` is `FINAL`.

## Panels

`AppShell.qml` is the root: title bar, nav rail, and a `SplitView` of
`SchedulePanel` — module — `LivePanel`. Module panels cross-fade rather than
cutting, so switching tools reads as one workspace rather than five apps.

`BiblePanel` `SongsPanel` `PicturesPanel` `MediaPanel` `AnnouncePanel` all bind
to `App`, the `Workspace` C++ object exposed as a context property.

## The C++ boundary

`Workspace` (`headers/workspace.hpp`) is a view-model and nothing more. It owns
the browsing state of each module and exposes it as properties.

It does **not** drive the projector. When the operator sends something live it
emits a request signal; `SoftProjector` — which owns the display screens,
themes and image generators — performs it and pushes the resulting live state
back in through `setLive*()`. That keeps the projection pipeline exactly where
it already was, and means a bug in the interface cannot put the wrong thing on
a screen in front of a congregation.

## Adding a module panel

1. Write `FooPanel.qml` with a `Toolbar` and one or more `Panel`s.
2. Add the browsing state to `Workspace` as properties plus a
   `requestFooLive(...)` signal.
3. Connect that signal in the `SoftProjector` constructor to whichever
   `set…List()` slot publishes the live state.
4. Add the file to `QML_FILES` in `src/CMakeLists.txt` and an entry to
   `shell.modules` in `AppShell.qml`.
