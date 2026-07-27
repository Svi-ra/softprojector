/***************************************************************************
//
//    SoftProjector - an open source media projection software
//
***************************************************************************/

#include "../headers/lucideprovider.hpp"

#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

LucideProvider::LucideProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
}

QByteArray LucideProvider::svgFor(const QString &name, const QString &rrggbb,
                                  const QString &strokeWidth)
{
    QByteArray src = mSource.value(name);
    if (src.isEmpty())
    {
        QFile f(QStringLiteral(":/lucide/icons/lucide/%1.svg").arg(name));
        if (!f.open(QIODevice::ReadOnly))
        {
            qWarning("LucideProvider: no such icon '%s'", qPrintable(name));
            return QByteArray();
        }
        src = f.readAll();
        mSource.insert(name, src);
    }

    src.replace("currentColor", ('#' + rrggbb).toLatin1());
    src.replace("stroke-width=\"2\"", "stroke-width=\"" + strokeWidth.toLatin1() + '"');
    return src;
}

QPixmap LucideProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    // <name>/<rrggbb>/<stroke>
    const QStringList parts = id.split('/');
    const QString name = parts.value(0);
    const QString rrggbb = parts.value(1, QStringLiteral("ffffff")).left(6);
    const QString stroke = parts.value(2, QStringLiteral("2.00"));

    const int edge = qMax(1, requestedSize.width() > 0 ? requestedSize.width() : 16);
    const QSize target(edge, requestedSize.height() > 0 ? requestedSize.height() : edge);

    QPixmap pm(target);
    pm.fill(Qt::transparent);

    const QByteArray svg = svgFor(name, rrggbb, stroke);
    if (!svg.isEmpty())
    {
        QSvgRenderer renderer(svg);
        if (renderer.isValid())
        {
            QPainter p(&pm);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setRenderHint(QPainter::SmoothPixmapTransform, true);
            renderer.render(&p, QRectF(QPointF(0, 0), QSizeF(target)));
        }
    }

    if (size)
        *size = target;
    return pm;
}
