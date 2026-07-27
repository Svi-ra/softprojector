/***************************************************************************
//
//    Lucide icon. Rendered from SVG at the exact pixel size and colour
//    requested, via the "lucide" image provider, so it stays crisp at any
//    device pixel ratio and needs no shader pass to recolour.
//
//    Usage:  Icon { name: "book-open"; color: Theme.textMuted }
//
***************************************************************************/

import QtQuick

Image {
    id: root

    property string name: ""
    property color color: Theme.text
    property int size: Theme.iconMd

    // Lucide strokes are authored at 2px on a 24px grid. Below ~18px that
    // reads as heavy, so thin it out to keep the set optically consistent.
    property real strokeWidth: size <= 14 ? 1.75 : (size <= 20 ? 2.0 : 2.25)

    width: size
    height: size

    sourceSize.width: size
    sourceSize.height: size
    smooth: true
    asynchronous: false
    cache: true
    fillMode: Image.PreserveAspectFit

    source: name === ""
            ? ""
            : "image://lucide/" + name + "/"
              + Qt.rgba(color.r, color.g, color.b, 1).toString().substring(1)
              + "/" + strokeWidth.toFixed(2)

    opacity: color.a

    Behavior on opacity {
        NumberAnimation { duration: Theme.dur(Theme.durFast); easing.type: Theme.easeOut }
    }
}
