/***************************************************************************
//
//    SoftProjector - an open source media projection software
//
//    Renders Lucide SVG icons on demand at an exact size, colour and stroke
//    width. Qt's SVG renderer does not understand `currentColor`, so the
//    attribute is substituted before parsing.
//
//    Image URL:  image://lucide/<icon-name>/<rrggbb>/<stroke-width>
//
***************************************************************************/

#ifndef LUCIDEPROVIDER_HPP
#define LUCIDEPROVIDER_HPP

#include <QQuickImageProvider>
#include <QHash>
#include <QByteArray>

class LucideProvider : public QQuickImageProvider
{
public:
    LucideProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QByteArray svgFor(const QString &name, const QString &rrggbb, const QString &strokeWidth);

    // Raw file contents, keyed by icon name. Icons are tiny (<1 KB) and the
    // same handful is drawn on every frame, so caching the source pays off.
    QHash<QString, QByteArray> mSource;
};

#endif // LUCIDEPROVIDER_HPP
