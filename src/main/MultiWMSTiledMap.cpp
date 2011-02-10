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

#include "MultiWMSTiledMap.h"

using namespace vesta;
using namespace std;


MultiWMSTiledMap::MultiWMSTiledMap(TextureMapLoader* loader,
                                   const QString& baseLayerName,
                                   unsigned int baseLayerLevelCount,
                                   const QString& detailLayerName,
                                   unsigned int detailLayerLevelCount,
                                   unsigned int tileSize) :
    HierarchicalTiledMap(loader, tileSize),
    m_baseLayerLevelCount(baseLayerLevelCount),
    m_detailLayerLevelCount(detailLayerLevelCount)
{
    m_baseTileNamePattern = QString("wms:") + baseLayerName + ",%1,%2,%3";
    m_detailTileNamePattern = QString("wms:") + detailLayerName + ",%1,%2,%3";
}


string
MultiWMSTiledMap::tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row)
{
    QString s;
    if (level < m_baseLayerLevelCount)
    {
        s = m_baseTileNamePattern.arg(level).arg(column).arg(row);
    }
    else
    {
        s = m_detailTileNamePattern.arg(level).arg(column).arg(row);
    }
    return string(s.toUtf8().data());
}


bool
MultiWMSTiledMap::isValidTileAddress(unsigned int level, unsigned int column, unsigned int row)
{
    return level < max(m_baseLayerLevelCount, m_detailLayerLevelCount) && column < (1u << (level + 1)) && row < (1u << level);
}


bool
MultiWMSTiledMap::tileResourceExists(const std::string& /* resourceId */)
{
    return true;//level < levelCount;
}
