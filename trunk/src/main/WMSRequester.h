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

class WMSRequester : public QObject
{
    Q_OBJECT

public:
    WMSRequester(QObject* parent = NULL);
    ~WMSRequester();

    struct LatLongBoundingBox
    {
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
        unsigned int level;
        unsigned int x;
        unsigned int y;
    };

    struct TileAssembly
    {
        QImage tileImage;
        int requestCount;
        QString layer;
        QString style;
        TileAddress address;
    };

    struct TileBuildOperation
    {
        TileAssembly* tile;
        QRectF subrect;
    };

    QString createWmsUrl(const QString& requestUrl,
                         const QString& layer,
                         const QString& style,
                         const LatLongBoundingBox& box,
                         unsigned int tileWidth,
                         unsigned int tileHeight) const;
    void retrieveTile(const QString& requestUrl,
                      const QString& layer,
                      const QString& style,
                      unsigned int level, unsigned int x, unsigned int y, unsigned int tileSize);

signals:
    void imageCompleted(const QImage& image);

private slots:
    void processTile(QNetworkReply* reply);

private:
    QString tileFileName(const QString& layer, const QString& style, const TileAddress& address);

private:
    QNetworkAccessManager* m_networkManager;
    QHash<QNetworkReply*, TileBuildOperation> m_pendingTiles;
};

#endif // _WMS_REQUESTER_H_
