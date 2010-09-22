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


WMSRequester::WMSRequester(QObject* parent) :
    QObject(parent),
    m_networkManager(NULL)
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
                           const QString& layer,
                           const QString& style,
                           const LatLongBoundingBox& box,
                           unsigned int tileWidth,
                           unsigned int tileHeight) const
{
    //QString requestBase = "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap";
    //QString layer = "BMNG";
    QString format = "image/jpeg";
    //QString style = "Nov";
    QString srs = "EPSG:4326";

    QString urlString = requestUrlBase;
    urlString += "&layers=" + layer;
    urlString += "&srs=" + srs;
    urlString += "&format=" + format;
    urlString += "&styles=" + style;
    urlString += "&width=" + QString::number(tileWidth) + "&height=" + QString::number(tileHeight);
    //urlString += "&bbox=" + QString::number(box.west) + "," + QString::number(box.south) + "," + QString::number(box.east) + "," + QString::number(box.north);
    urlString += QString("&bbox=%1,%2,%3,%4").arg(box.west).arg(box.south).arg(box.east).arg(box.north);

    return urlString;
}


void
WMSRequester::retrieveTile(const QString& requestUrl,
                           const QString& layer,
                           const QString& style,
                           unsigned int level, unsigned int x, unsigned int y, unsigned int tileSize)
{
    unsigned int levelHeight = (1 << level) * tileSize;
    unsigned int levelWidth = levelHeight * 2;

    unsigned int nativeWmsLevelWidth = 86400;
    unsigned int wmsTileWidth = 480;
    unsigned int wmsTileHeight = 480;

    double tileExtent = 180.0 / double(1 << level);
    LatLongBoundingBox tile;
    tile.west = -180.0 + x * tileExtent;
    tile.south = -90.0 + y * tileExtent;
    tile.east = tile.west + tileExtent;
    tile.north = tile.south + tileExtent;

    LatLongBoundingBox topLeft;
    topLeft.west = -180.0;
    topLeft.south = -166.0;
    topLeft.east = 76.0;
    topLeft.north = 90.0;

    double requestedResolution = 360.0 / levelWidth;
    double baseWmsResolution = (topLeft.east - topLeft.west) / double(wmsTileWidth);

    unsigned int wmsLevel = 0;
    while (requestedResolution < baseWmsResolution / (1 << wmsLevel))
    {
        wmsLevel++;
    }

    //qDebug() << "retrieveTile: " << wmsLevel << ", " << tile.west << "," << tile.south << "," << tile.east << "," << tile.north;

    double wmsTileLonExtent = (topLeft.east - topLeft.west) / (1 << wmsLevel);
    double wmsTileLatExtent = (topLeft.north - topLeft.south) / (1 << wmsLevel);
    topLeft.east = topLeft.west + wmsTileLonExtent;
    topLeft.south = topLeft.north - wmsTileLatExtent;

    int westIndex  = int(floor((tile.west  - topLeft.west) / wmsTileLonExtent));
    int southIndex = int(floor((tile.south - topLeft.south) / wmsTileLatExtent));
    int eastIndex  = int(ceil((tile.east  - topLeft.west) / wmsTileLonExtent));
    int northIndex = int(ceil((tile.north - topLeft.south) / wmsTileLatExtent));

    TileAssembly* tileAssembly = new TileAssembly;
    tileAssembly->tileImage = QImage(tileSize, tileSize, QImage::Format_RGB888);
    tileAssembly->requestCount = 0;
    tileAssembly->layer = layer;
    tileAssembly->style = style;
    tileAssembly->address.level = level;
    tileAssembly->address.x = x;
    tileAssembly->address.y = y;

    for (int lat = southIndex; lat < northIndex; ++lat)
    {
        for (int lon = westIndex; lon < eastIndex; ++lon)
        {
            LatLongBoundingBox bbox;
            bbox.west  = topLeft.west + lon * wmsTileLonExtent;
            bbox.south = topLeft.south + lat * wmsTileLatExtent;
            bbox.east = bbox.west + wmsTileLonExtent;
            bbox.north = bbox.south + wmsTileLatExtent;

            QString urlString = createWmsUrl(requestUrl, layer, style, bbox, 480, 480);
            qDebug() << urlString;

            QUrl url(urlString);
            QNetworkRequest request(url);
            //QNetworkCacheMetaData metadata = m_networkManager->cache()->metaData(url);
            //qDebug() << "metadata: " << metadata.attributes() << ", " << metadata.expirationDate().toString();
            request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
            QNetworkReply* reply = m_networkManager->get(request);

            TileBuildOperation op;
            op.tile = tileAssembly;
            op.subrect = QRectF(float(tileSize * (bbox.west - tile.west) / tileExtent),
                                -float(tileSize * (bbox.north - tile.north) / tileExtent),
                                tileSize * wmsTileLonExtent / tileExtent, tileSize * wmsTileLatExtent / tileExtent);
            m_pendingTiles[reply] = op;

            tileAssembly->requestCount++;
        }
    }

    // Pattern example:
    // request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Dec_nb&width=480&height=480&bbox=-180,-166,76,90]
    // request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Dec_nb&width=480&height=480&bbox=-180,-38,-52,90]
    // ...
    // request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Dec_nb&width=480&height=480&bbox=-180,88,-178,90]
}


void
WMSRequester::processTile(QNetworkReply* reply)
{
    // Check HTTP status code
    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    // Check redirection target
    QVariant redirectionTargetUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    // see CS001432 on how to handle this

    // no error received?
    if (reply->error() == QNetworkReply::NoError)
    {
        bool fromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();

        QImageReader imageReader(reply);
        QImage image = imageReader.read();

        qDebug() << "image from cache?" << fromCache << ", size: " << image.size();
        if (m_pendingTiles.contains(reply))
        {
            TileBuildOperation op = m_pendingTiles.value(reply);
            TileAssembly* tileAssembly = op.tile;

            QPainter painter(&tileAssembly->tileImage);
            painter.drawImage(op.subrect, image);
            painter.end();
            m_pendingTiles.remove(reply);

            tileAssembly->requestCount--;
            qDebug() << tileAssembly->requestCount << " remaining";
            if (tileAssembly->requestCount == 0)
            {
                QString imageName = tileFileName(tileAssembly->layer, tileAssembly->style, tileAssembly->address);
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

                emit imageCompleted(tileAssembly->tileImage);
                delete tileAssembly;
            }
        }
    }
    else
    {
        // Oops... error
    }

    //delete reply;
}


QString
WMSRequester::tileFileName(const QString& layer, const QString& style, const TileAddress& address)
{
    QString cacheDirName = QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + "/wms_tiles";
    return QString("%1/%2_%3/tile_%4_%5_%6.png").arg(cacheDirName).arg(layer).arg(style).
                                                 arg(address.level).arg(address.y).arg(address.x);
}
