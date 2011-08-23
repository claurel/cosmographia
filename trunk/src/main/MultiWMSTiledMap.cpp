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
#include <algorithm>

using namespace vesta;
using namespace std;


MultiWMSTiledMap::MultiWMSTiledMap(TextureMapLoader* loader,
                                   const QString& baseLayerName,
                                   unsigned int baseLayerMaxLevel,
                                   const QString& detailLayerName,
                                   unsigned int detailLayerMaxLevel,
                                   unsigned int tileSize) :
    HierarchicalTiledMap(loader, tileSize)
{
    QString baseNamePattern = QString("wms:") + baseLayerName + ",%1,%2,%3";
    QString tileNamePattern = QString("wms:") + detailLayerName + ",%1,%2,%3";

    setupLevelRanges("", baseNamePattern, baseLayerMaxLevel, tileNamePattern, detailLayerMaxLevel);
}


MultiWMSTiledMap::MultiWMSTiledMap(TextureMapLoader* loader,
                                   const QString& topLayerName,
                                   const QString& baseLayerName,
                                   unsigned int baseLayerMaxLevel,
                                   const QString& detailLayerName,
                                   unsigned int detailLayerMaxLevel,
                                   unsigned int tileSize) :
    HierarchicalTiledMap(loader, tileSize)
{
    QString baseNamePattern = QString("wms:") + baseLayerName + ",%1,%2,%3";
    QString tileNamePattern = QString("wms:") + detailLayerName + ",%1,%2,%3";

    setupLevelRanges(topLayerName, baseNamePattern, baseLayerMaxLevel, tileNamePattern, detailLayerMaxLevel);
}


void
MultiWMSTiledMap::setupLevelRanges(const QString& topLayerPattern,
                                   const QString& baseLayerPattern,
                                   unsigned int baseLayerMaxLevel,
                                   const QString& detailLayerPattern,
                                   unsigned int detailLayerMaxLevel)
{
    bool hasTopLayer = !topLayerPattern.isEmpty();

    unsigned int baseLayerLevelCount = baseLayerMaxLevel;
    unsigned int detailLayerLevelCount = detailLayerMaxLevel > baseLayerMaxLevel ? detailLayerMaxLevel - baseLayerMaxLevel : 0;

    if (hasTopLayer)
    {
        TileLevelRange top;
        top.levelCount = 1;
        top.tileNamePattern = topLayerPattern;
        m_tileLevelRanges << top;

        baseLayerLevelCount = std::max(1u, baseLayerLevelCount) - 1;
    }

    if (baseLayerLevelCount > 0)
    {
        TileLevelRange base;
        base.levelCount = baseLayerLevelCount;
        base.tileNamePattern = baseLayerPattern;
        m_tileLevelRanges << base;
    }

    if (detailLayerLevelCount > 0)
    {
        TileLevelRange detail;
        detail.levelCount = detailLayerLevelCount;
        detail.tileNamePattern = detailLayerPattern;
        m_tileLevelRanges << detail;
    }
}


string
MultiWMSTiledMap::tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row)
{
    QString s;

    unsigned int count = 0;
    foreach (TileLevelRange range, m_tileLevelRanges)
    {
        count += range.levelCount;
        if (level < count)
        {
            s = range.tileNamePattern.arg(level).arg(column).arg(row);
            break;
        }
    }

    return string(s.toUtf8().data());
}


bool
MultiWMSTiledMap::isValidTileAddress(unsigned int level, unsigned int column, unsigned int row)
{
    unsigned int count = 0;
    foreach (TileLevelRange range, m_tileLevelRanges)
    {
        count += range.levelCount;
    }

    return level < count && column < (1u << (level + 1)) && row < (1u << level);
}


bool
MultiWMSTiledMap::tileResourceExists(const std::string& /* resourceId */)
{
    return true;//level < levelCount;
}
