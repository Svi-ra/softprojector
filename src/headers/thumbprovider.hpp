/***************************************************************************
//
//    SoftProjector - an open source media projection software
//
//    Serves slide thumbnails to QML. Pixmaps come out of the database as
//    BLOBs, so there is no file path to point an Image at; they are parked
//    here under a key instead.
//
//    Image URL:  image://spthumb/<key>
//
***************************************************************************/

#ifndef THUMBPROVIDER_HPP
#define THUMBPROVIDER_HPP

#include <QQuickImageProvider>
#include <QHash>
#include <QPixmap>

class ThumbProvider : public QQuickImageProvider
{
public:
    ThumbProvider();

    static ThumbProvider *instance();

    void put(const QString &key, const QPixmap &pixmap);
    void clear();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QHash<QString, QPixmap> mThumbs;
};

#endif // THUMBPROVIDER_HPP
