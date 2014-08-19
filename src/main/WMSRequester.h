// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _WMS_REQUESTER_H_
#define _WMS_REQUESTER_H_

#include <vesta/TextureMap.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QImage>
#include <QMutex>

class WMSRequester : public QObject
{
    Q_OBJECT

public:
    WMSRequester(QObject* parent);
    ~WMSRequester();

    struct LatLongBoundingBox
    {
        LatLongBoundingBox() :
            west(0.0), south(0.0), east(0.0), north(0.0)
        {
        }

        LatLongBoundingBox(double _west, double _south, double _east, double _north) :
            west(_west), south(_south), east(_east), north(_north)
        {
        }

        QRectF toRect()
        {
            return QRectF(float(west), float(south), float(east - west), float(north - south));
        }

        double west;
        double south;
        double east;
        double north;
    };

    struct TiledGroup
    {
        QString name;
        QString title;
        QString abstract;
        unsigned int pad;
        unsigned int bandCount;
        LatLongBoundingBox bbox;
    };

    struct TileAddress
    {
        bool valid;
        QString surface;
        unsigned int level;
        unsigned int x;
        unsigned int y;
    };

    struct TileAssembly
    {
        QString tileName;
        QString surfaceName;
        QImage tileImage;
        unsigned int tileWidth;
        unsigned int tileHeight;
        int requestCount;
        TileAddress address;
        vesta::TextureMap* texture;
    };

    struct SurfaceProperties
    {
        QString requestUrl;
        unsigned int tileWidth;
        unsigned int tileHeight;
        LatLongBoundingBox topLeft;
    };

    struct TileBuildOperation
    {
        TileBuildOperation() : tile(NULL), subrect() {}
        TileBuildOperation(const TileBuildOperation& other) :
            tile(other.tile),
            subrect(other.subrect),
            urlString(other.urlString)
        {
        }

        TileBuildOperation& operator=(const TileBuildOperation& other)
        {
            tile = other.tile;
            subrect = other.subrect;
            urlString = other.urlString;
            return *this;
        }

        TileAssembly* tile;
        QRectF subrect;
        QString urlString;
    };

    void addSurfaceDefinition(const QString& name,
                              const QString& requestBase,
                              const LatLongBoundingBox& topLeftBox,
                              unsigned int tileWidth, unsigned int tileHeight);

    static TileAddress parseTileName(const QString& tileName);

    unsigned int pendingTileCount() const;

public slots:
    void retrieveTile(const QString& tileName,
                      const QString& surface,
                      const QRectF& tileRect,
                      unsigned int tileSize,
                      vesta::TextureMap* texture);

private slots:
    void processTile(QNetworkReply* reply);

signals:
    void imageCompleted(const QString& tileName, const QImage& image);

private:
    QString tileFileName(const QString& tileName, const QString& surfaceName);
    QString createWmsUrl(const QString& requestUrl,
                         const LatLongBoundingBox& box,
                         unsigned int tileWidth,
                         unsigned int tileHeight) const;
    void requestTile(const TileBuildOperation& op);

private:
    QNetworkAccessManager* m_networkManager;
    QList<TileBuildOperation> m_queuedTiles;
    QHash<QNetworkReply*, TileBuildOperation> m_requestedTiles;
    QHash<QString, SurfaceProperties> m_surfaces;
    QMutex m_mutex;
    int m_dispatchedRequestCount;
};

#endif // _WMS_REQUESTER_H_
