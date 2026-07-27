/***************************************************************************
//
//    SoftProjector - an open source media projection software
//
***************************************************************************/

#include "../headers/thumbprovider.hpp"

static ThumbProvider *gThumbProvider = nullptr;

ThumbProvider::ThumbProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
    gThumbProvider = this;
}

ThumbProvider *ThumbProvider::instance()
{
    // Created once and handed to the QML engine, which takes ownership.
    if (!gThumbProvider)
        new ThumbProvider;
    return gThumbProvider;
}

void ThumbProvider::put(const QString &key, const QPixmap &pixmap)
{
    mThumbs.insert(key, pixmap);
}

void ThumbProvider::clear()
{
    mThumbs.clear();
}

QPixmap ThumbProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QPixmap pm = mThumbs.value(id);
    if (pm.isNull())
    {
        pm = QPixmap(requestedSize.isValid() ? requestedSize : QSize(64, 64));
        pm.fill(Qt::transparent);
    }
    else if (requestedSize.width() > 0 && requestedSize.height() > 0)
    {
        pm = pm.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if (size)
        *size = pm.size();
    return pm;
}
