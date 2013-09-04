// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// This file is a part of qtvesta, a set of classes for using
// the VESTA library with the Qt framework.
//
// qtvesta is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// qtvesta is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with qtvesta. If not, see <http://www.gnu.org/licenses/>.

#include "WMSRequester.h"
#include <QNetworkDiskCache>
#include <QDesktopServices>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QNetworkRequest>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <cmath>

using namespace std;

// Maximum number of requests for the QNetworkAccessManager. The WMSRequester
// class will take care of managing the other pending requests itself.
const static int MaxOutstandingNetworkRequests = 12;


/** WMSRequester handles retrieving map tiles from a Web Map Server and converting
  * them to a form that can be easily used with VESTA's WorldGeometry class. The
  * tiles are converted to a power-of-two sizes that can be used by any GPU.
  *
  * WorldGeometry expects a 'pyramid' of map tiles to cover the globe. The top
  * level of the pyramid is a 2x1 grid of tiles. Lower levels contain 4x2, 8x4, ...
  * tiles. A WMS server can deliver tiles that work with this scheme, but JPL's
  * OnEarth server is often overloaded. Tiles can only be retrieved reliably from
  * OnEarth when using a restricted set of requests described by GetTileService:
  *
  * http://onearth.jpl.nasa.gov/tiled.html
  *
  * The tiles available through TiledPattern accesses is more limited, and they do
  * not generally work well with WorldGeometry. The WMSRequester takes care of
  * assembling multiple tiles from a WMS server into tiles for WorldGeometry. The
  * WorldGeometry tiles are constructed with one or more blits, which may or may
  * not also scale the image.
  *
  * A request to make a texture tile resident spawns one or more server requests.
  * As image tiles come back from the WMS server, they are blitted to a QImage. When
  * then final tile is received, WMSRequester emits an imageCompleted signal that
  * the tile is ready to be converted to a texture.
  *
  * Requests are sent directly to Qt's NetworkAccessManager until a limit is reached.
  * At that point requests are added to a queued tiles list. While the
  * NetworkAccessManager does handle queuing itself (restricting the maximum
  * number of simultaneous HTTP connections to 6), we need more control over the
  * queue:
  *    - Currently visible tiles should have priority, and are moved to the front
  *      of the queue.
  *    - If the user moves the camera quickly over the surface of a planet, huge
  *      number of requests may be queued. We occasionally trim queue, removing
  *      requests for tiles that haven't been visible for some time.
  */
WMSRequester::WMSRequester(QObject* parent) :
    QObject(parent),
    m_dispatchedRequestCount(0)
{
    m_networkManager = new QNetworkAccessManager(this);
    QNetworkDiskCache* cache = new QNetworkDiskCache(this);
    //cache->setCacheDirectory(QDesktopServices::storageLocation(QDesktopServices::CacheLocation));
    cache->setCacheDirectory(QStandardPaths::locate(QStandardPaths::CacheLocation, ""));
    m_networkManager->setCache(cache);
    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processTile(QNetworkReply*)));
}


WMSRequester::~WMSRequester()
{
}


/** Create the URL for a WMS request. Most of the parameters are included in the
  * requestUrlBase. This method only adds parameters for the bounding box and
  * tile size.
  */
QString
WMSRequester::createWmsUrl(const QString& requestUrlBase,
                           const LatLongBoundingBox& box,
                           unsigned int tileWidth,
                           unsigned int tileHeight) const
{
    QString urlString = requestUrlBase;
    urlString += "&width=" + QString::number(tileWidth) + "&height=" + QString::number(tileHeight);
    urlString += QString("&bbox=%1,%2,%3,%4").arg(box.west,  0, 'f', 6).
                                              arg(box.south, 0, 'f', 6).
                                              arg(box.east,  0, 'f', 6).
                                              arg(box.north, 0, 'f', 6);

    return urlString;
}


/** Retrieve the named tile from the WMS server. A tileCompleted signal will
  * be emitted when the tile is ready.
  */
void
WMSRequester::retrieveTile(const QString& tileName,
                           const QString& surface,
                           const QRectF& tileRect,
                           unsigned int tileSize,
                           vesta::TextureMap* texture)
{
    if (!m_surfaces.contains(surface))
    {
        // Surface not defined
        return;
    }

    QString fileName = tileFileName(tileName, surface);
    QFileInfo fileInfo(fileName);
    if (fileInfo.exists())
    {
        QImage image(fileName);
        emit imageCompleted(tileName, image);
        return;
    }

    LatLongBoundingBox tileBox(tileRect.x(),
                               tileRect.y(),
                               tileRect.x() + tileRect.width(),
                               tileRect.y() + tileRect.height());
    SurfaceProperties surfaceProps = m_surfaces[surface];
    unsigned int wmsTileWidth = surfaceProps.tileWidth;
    unsigned int wmsTileHeight = surfaceProps.tileHeight;
    LatLongBoundingBox topLeft = surfaceProps.topLeft;

    double tileLongExtent = tileBox.east  - tileBox.west;
    double tileLatExtent  = tileBox.north - tileBox.south;

    double requestedResolution = tileLongExtent / double(tileSize);
    double baseWmsResolution = (topLeft.east - topLeft.west) / double(wmsTileWidth);

    unsigned int wmsLevel = 0;
    while (requestedResolution < baseWmsResolution / (1 << wmsLevel))
    {
        wmsLevel++;
    }

    double wmsTileLongExtent = (topLeft.east - topLeft.west) / (1 << wmsLevel);
    double wmsTileLatExtent = (topLeft.north - topLeft.south) / (1 << wmsLevel);
    topLeft.east = topLeft.west + wmsTileLongExtent;
    topLeft.south = topLeft.north - wmsTileLatExtent;

    int westIndex  = int(floor((tileBox.west  - topLeft.west) / wmsTileLongExtent));
    int southIndex = int(floor((tileBox.south - topLeft.south) / wmsTileLatExtent));
    int eastIndex  = int(ceil((tileBox.east  - topLeft.west) / wmsTileLongExtent));
    int northIndex = int(ceil((tileBox.north - topLeft.south) / wmsTileLatExtent));

    TileAssembly* tileAssembly = new TileAssembly;
    tileAssembly->tileImage;
    tileAssembly->requestCount = 0;
    tileAssembly->tileName = tileName;
    tileAssembly->surfaceName = surface;
    tileAssembly->tileWidth = tileSize;
    tileAssembly->tileHeight = tileSize;
    tileAssembly->texture = texture;

    for (int lat = southIndex; lat < northIndex; ++lat)
    {
        for (int lon = westIndex; lon < eastIndex; ++lon)
        {
            LatLongBoundingBox bbox;
            bbox.west  = topLeft.west + lon * wmsTileLongExtent;
            bbox.south = topLeft.south + lat * wmsTileLatExtent;
            bbox.east = bbox.west + wmsTileLongExtent;
            bbox.north = bbox.south + wmsTileLatExtent;

            QString urlString = createWmsUrl(surfaceProps.requestUrl, bbox, surfaceProps.tileWidth, surfaceProps.tileHeight);

            TileBuildOperation op;
            op.tile = tileAssembly;
            op.subrect = QRectF(float(tileSize * (bbox.west - tileBox.west) / tileLongExtent),
                                -float(tileSize * (bbox.north - tileBox.north) / tileLatExtent),
                                tileSize * wmsTileLongExtent / tileLongExtent, tileSize * wmsTileLatExtent / tileLatExtent);
            op.urlString = urlString;
            op.tile->requestCount++;

            if (m_dispatchedRequestCount < MaxOutstandingNetworkRequests)
            {
                requestTile(op);
            }
            else
            {
                QMutexLocker locker(&m_mutex);
                m_queuedTiles.append(op);
            }
        }
    }
}


void
WMSRequester::requestTile(const TileBuildOperation& op)
{
    QUrl url(op.urlString);

    // Testing the 'SourceIsFromCache' attribute of replies from the OnEarth server
    // seems to indicate that the tiles are not being cached. However, tiles are still
    // appearing in the cache directory. The following code attempts to force using
    // the cache, but may not be effective.
    QNetworkCacheMetaData cacheData = m_networkManager->cache()->metaData(url);
    QNetworkRequest request(url);
    if (cacheData.isValid())
    {
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
    }
    else
    {
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    }

    QNetworkReply* reply = m_networkManager->get(request);

    m_mutex.lock();
    m_dispatchedRequestCount++;
    m_requestedTiles[reply] = op;
    m_mutex.unlock();
}


// Private slot to take care of processing a WMS tile delivered over the network.
void
WMSRequester::processTile(QNetworkReply* reply)
{
    // Check HTTP status code
    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    // Check redirection target
    QVariant redirectionTargetUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    // see CS001432 on how to handle this

    --m_dispatchedRequestCount;

    // no error received?
    if (reply->error() == QNetworkReply::NoError)
    {
        bool fromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();

        QImageReader imageReader(reply);
        QImage image = imageReader.read();

        if (image.isNull())
        {
            qDebug() << "Received bad image: " << reply->header(QNetworkRequest::LocationHeader);
        }
        else
        {
            QMutexLocker locker(&m_mutex);

            TileBuildOperation op = m_requestedTiles.take(reply);

            TileAssembly* tileAssembly = op.tile;

            if (tileAssembly)
            {
                bool firstOp = false;
                if (tileAssembly->tileImage.isNull())
                {
                     tileAssembly->tileImage = QImage(tileAssembly->tileWidth, tileAssembly->tileHeight, QImage::Format_RGB888);
                     firstOp = true;
                }

                QPainter painter(&tileAssembly->tileImage);

                // Clear the background to white before the first operation
                if (firstOp)
                {
                    painter.fillRect(QRectF(0.0f, 0.0f, tileAssembly->tileImage.width(), tileAssembly->tileImage.height()), Qt::white);
                }

                painter.setRenderHints(QPainter::SmoothPixmapTransform, true);

                // A hack to work around some drawing problems that left occasional gaps
                // in tiles. There's either a bug in Qt's painter class, or some trouble
                // with roundoff errors. Increasing the rectangle size very slightly
                // eliminates the gaps.
                QRectF r(op.subrect);
                r.setSize(QSizeF(r.width() * 1.0001f, r.height() * 1.0001f));
                painter.drawImage(r, image);
                painter.end();

                tileAssembly->requestCount--;
                if (tileAssembly->requestCount == 0)
                {
                    QString imageName = tileFileName(tileAssembly->tileName, tileAssembly->surfaceName);
                    QFileInfo fileInfo(imageName);
                    QDir tileDir = fileInfo.dir();
                    if (!tileDir.exists())
                    {
                        tileDir.mkpath(tileDir.absolutePath());
                    }

                    bool ok = tileAssembly->tileImage.save(imageName);
                    if (!ok)
                    {
                        qDebug() << "Failed writing to " << imageName;
                    }

                    emit imageCompleted(tileAssembly->tileName, tileAssembly->tileImage.rgbSwapped());
                    delete tileAssembly;
                }
            }
        }
    }
    else
    {
        qDebug() << "Network error: " << reply->errorString();
    }

    reply->deleteLater();

    // If there are queued tiled requests and not too many active WMS server connections,
    // then make some more network requests. Prioritize requests for textures tiles that
    // are currently visible.
    while (m_dispatchedRequestCount < MaxOutstandingNetworkRequests)
    {
        int tileIndex = -1;
        vesta::v_uint64 mostRecent = 0;
        TileBuildOperation op;
        {
            QMutexLocker locker(&m_mutex);

            // Request a WMS tile for the most recently used texture tile
            for (int i = 0; i < m_queuedTiles.size(); ++i)
            {
                if (m_queuedTiles[i].tile->texture->lastUsed() > mostRecent)
                {
                    tileIndex = i;
                    mostRecent = m_queuedTiles[i].tile->texture->lastUsed();
                }
            }

            if (tileIndex >= 0)
            {
                op = m_queuedTiles.takeAt(tileIndex);

                // Trim the queue by removing requests for tiles that haven't been visible
                // for a while. This will happen when the user moves the camera quickly over
                // the surface of a planet. We want to load tiles for the location that the
                // user is looking at now, not the places that they zoomed past quickly on
                // the way there.
                //
                // Tiles that haven't been accessed in the last cullLag frames are remove
                const unsigned int cullLag = 60;
                if (mostRecent >= cullLag)
                {
                    vesta::v_uint64 cullBefore = mostRecent - cullLag;
                    for (int i = m_queuedTiles.size() - 1; i >= 0; --i)
                    {
                        if (m_queuedTiles[i].tile->texture->lastUsed() < cullBefore)
                        {
                            // Set status to unitialized so that loading will be retried if the tile
                            // comes into view later.
                            m_queuedTiles[i].tile->texture->setStatus(vesta::TextureMap::Uninitialized);
                            m_queuedTiles.removeAt(i);
                        }
                    }
                }
            }
        }

        if (tileIndex >= 0)
        {
            requestTile(op);
        }
        else
        {
            break;
        }
    }
}


void
WMSRequester::addSurfaceDefinition(const QString& name,
                                   const QString& requestBase,
                                   const LatLongBoundingBox& topLeftBox,
                                   unsigned int tileWidth, unsigned int tileHeight)
{
    SurfaceProperties surface;
    surface.requestUrl = requestBase;
    surface.tileWidth = tileWidth;
    surface.tileHeight = tileHeight;
    surface.topLeft = topLeftBox;
    m_surfaces[name] = surface;
}


QString
WMSRequester::tileFileName(const QString& tileName, const QString& surfaceName)
{
    //QString cacheDirName = QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + "/wms_tiles";
    QString cacheDirName = QStandardPaths::locate(QStandardPaths::CacheLocation, "wms_tiles", QStandardPaths::LocateDirectory);
    return QString("%1/%2/%3.png").arg(cacheDirName).arg(surfaceName).arg(tileName);
}


// The names should all have the form:
//   wms:LAYERNAME:LEVEL:X:Y
// For example, wms:earth-bmng:3:7:1
WMSRequester::TileAddress
WMSRequester::parseTileName(const QString& tileName)
{
    QString baseName = tileName;
    baseName = baseName.remove("wms:");
    QStringList nameParts = baseName.split(",");
    bool nameOk = false;

    QString surfaceName;
    int level = 0;
    int x = 0;
    int y = 0;

    if (nameParts.size() == 4)
    {
        surfaceName = nameParts[0];
        bool layerOk = false;
        bool xOk = false;
        bool yOk = false;

        level = nameParts[1].toInt(&layerOk);
        x = nameParts[2].toInt(&xOk);
        y = nameParts[3].toInt(&yOk);

        nameOk = layerOk && xOk && yOk && level >= 0 && x >= 0 && y >= 0;
    }

    TileAddress address;
    address.valid = nameOk;
    if (address.valid)
    {
        address.surface = surfaceName;
        address.level = level;
        address.x = x;
        address.y = y;
    }

    return address;
}


/** Get the number of tiles in the queue. The count includes both active
  * and queued requests. Since different texture tiles may use the same
  * WMS tiles, the count will generally be larger (often by a factor of
  * four or more) than the number of texture tiles.
  */
unsigned int
WMSRequester::pendingTileCount() const
{
    return (unsigned int) (m_dispatchedRequestCount + m_queuedTiles.size());
}
