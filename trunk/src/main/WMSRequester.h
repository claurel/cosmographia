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

#ifndef _WMS_REQUESTER_H_
#define _WMS_REQUESTER_H_

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
        TileAssembly* tile;
        QRectF subrect;
    };

    void addSurfaceDefinition(const QString& name,
                              const QString& requestBase,
                              const LatLongBoundingBox& topLeftBox,
                              unsigned int tileWidth, unsigned int tileHeight);

    static TileAddress parseTileName(const QString& tileName);

public slots:
    void retrieveTile(const QString& tileName,
                      const QString& surface,
                      const QRectF& tileRect,
                      unsigned int tileSize);

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

private:
    QNetworkAccessManager* m_networkManager;
    QHash<QNetworkReply*, TileBuildOperation> m_pendingTiles;
    QHash<QString, SurfaceProperties> m_surfaces;
    QMutex m_mutex;
};

#endif // _WMS_REQUESTER_H_
