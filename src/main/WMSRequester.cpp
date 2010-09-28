// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

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

const static int MaxOutstandingNetworkRequests = 12;


WMSRequester::WMSRequester(QObject* parent) :
    QObject(parent),
    m_dispatchedRequestCount(0)
{
    m_networkManager = new QNetworkAccessManager(this);
    QNetworkDiskCache* cache = new QNetworkDiskCache(this);
    cache->setCacheDirectory(QDesktopServices::storageLocation(QDesktopServices::CacheLocation));
    m_networkManager->setCache(cache);
    qDebug() << "cache location: " << cache->cacheDirectory();
    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processTile(QNetworkReply*)));
}


WMSRequester::~WMSRequester()
{
}


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
    //qDebug() << "requesting: " << op.urlString;

    QUrl url(op.urlString);
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

            //bool wasCached = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();
            TileAssembly* tileAssembly = op.tile;

            if (tileAssembly)
            {
                if (tileAssembly->tileImage.isNull())
                {
                     tileAssembly->tileImage = QImage(tileAssembly->tileWidth, tileAssembly->tileHeight, QImage::Format_RGB888);
                }

                QPainter painter(&tileAssembly->tileImage);
                painter.drawImage(op.subrect, image);
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

    while (m_dispatchedRequestCount < MaxOutstandingNetworkRequests)
    {
        int tileIndex = -1;
        vesta::v_uint64 mostRecent = 0;
        TileBuildOperation op;
        {
            QMutexLocker locker(&m_mutex);
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

                // Tiles that haven't been accessed in the last cullLag frames are remove
                const unsigned int cullLag = 100;
                if (mostRecent >= cullLag)
                {
                    vesta::v_uint64 cullBefore = mostRecent - cullLag;
                    for (int i = m_queuedTiles.size() - 1; i >= 0; --i)
                    {
                        if (m_queuedTiles[i].tile->texture->lastUsed() < cullBefore)
                        {
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
    QString cacheDirName = QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + "/wms_tiles";
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


unsigned int
WMSRequester::pendingTileCount() const
{
    return (unsigned int) (m_dispatchedRequestCount + m_queuedTiles.size());
}
