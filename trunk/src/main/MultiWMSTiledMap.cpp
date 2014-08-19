// This file is part of Cosmographia.
//
// Copyright (C) 2010-2011 Chris Laurel <claurel@gmail.com>
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
