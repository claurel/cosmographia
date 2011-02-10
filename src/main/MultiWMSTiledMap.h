// This file is part of Cosmographia.
//
// Copyright (C) 2010-2011 Chris Laurel <claurel@gmail.com>
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

#ifndef _MULTI_WMS_TILED_MAP_H_
#define _MULTI_WMS_TILED_MAP_H_

#include <vesta/HierarchicalTiledMap.h>
#include <QString>
#include <string>


class MultiWMSTiledMap : public vesta::HierarchicalTiledMap
{
public:
    MultiWMSTiledMap(vesta::TextureMapLoader* loader,
                     const QString& baseLayerName,
                     unsigned int baseLayerLevelCount,
                     const QString& detailLayerName,
                     unsigned int detailLayerLevelCount,
                     unsigned int tileSize);

    virtual std::string tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row);
    virtual bool isValidTileAddress(unsigned int level, unsigned int column, unsigned int row);
    virtual bool tileResourceExists(const std::string& resourceId);

private:
    QString m_baseTileNamePattern;
    QString m_detailTileNamePattern;
    unsigned int m_baseLayerLevelCount;
    unsigned int m_detailLayerLevelCount;
};

#endif // _MULTI_WMS_TILED_MAP_H_
