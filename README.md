# softprojector

An open source media projection application for churches: Bible readings, songs,
announcements, images and video, sent to one or more projector screens.

The operator workspace is a Qt Quick (QML) interface; the projection pipeline,
database access, themes and the secondary dialogs remain C++/Qt Widgets.

## Building

Requires **Qt 6.5+** (Quick, QuickWidgets, Multimedia, Sql, Svg, PrintSupport)
and a C++17 compiler. The build is CMake-based.

```bash
cmake -S src -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/Qt/6.8.3/<compiler>
cmake --build build
```

The binary lands in `build/bin/`. On Windows, make it redistributable with:

```bash
windeployqt --release --qmldir src/qml build/bin/SoftProjector.exe
```

`spData.sqlite` is created next to the executable on first run if it is missing.

## Interface

The workspace is laid out left to right, and the outer columns never move, so
the operator's hands always know where the running order and Show are:

| Region | What it is |
| --- | --- |
| Navigation rail | The five modules — Bible, Songs, Images, Media, Notices — plus manage data, settings, and the light/dark toggle |
| Schedule | The running order for the meeting, with reordering and save/open |
| Module | The active module's browsing and preview panels |
| Live | What is on the projector right now, the slides to step through, and Show / Hide / Clear |

Keyboard: `F6`–`F10` switch modules, `PgUp`/`PgDn` step slides, `Shift+F5`
shows, `Ctrl+F` focuses search, `Ctrl+D` toggles the theme, and the usual
`Ctrl+N` / `O` / `S` / `P` apply to the schedule.

## Layout of the source

```
src/
  qml/          QML workspace: design tokens, component library, module panels
  headers/      C++ headers
  sources/      C++ sources
  ui/           Qt Designer forms for the remaining widget dialogs
  icons/lucide/ The Lucide icons the interface draws
```

See [`src/qml/README.md`](src/qml/README.md) for the design system and how to
add a component or a module panel.

## Licence

GPL v3. Lucide icons are ISC licensed — see `src/icons/lucide/LICENSE`.
