// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
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

#include "LocalTiledMap.h"
#include <QFileInfo>

using namespace vesta;
using namespace std;


/** \param pattern is a string that will be used to construct a tile name given
  *        the level, column, and row. %1, %2, and %3 in the string will be replaced
  *        with the values of the level, column, and row, respectively.
  *
  * Example pattern: "mars/level%1/tile_%2_%3.png"
  */
LocalTiledMap::LocalTiledMap(TextureMapLoader* loader, const QString& tileNamePattern, bool flipped, unsigned int tileSize, unsigned int levelCount) :
    HierarchicalTiledMap(loader, tileSize),
    m_tileNamePattern(tileNamePattern),
    m_flipped(flipped),
    m_levelCount(levelCount)
{
}


string
LocalTiledMap::tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row)
{
    // Row may be inverted here if the tiles are arranged so that the northernmost
    // tile in a level is at row 0.
    unsigned int y = m_flipped ? (1 << level) - 1 - row : row;
    QString s = m_tileNamePattern.arg(level).arg(column).arg(y);
    return string(s.toUtf8().data());
}


bool
LocalTiledMap::isValidTileAddress(unsigned int level, unsigned int column, unsigned int row)
{
    return level < m_levelCount && column < (1u << (level + 1)) && row < (1u << level);
}


bool
LocalTiledMap::tileResourceExists(const std::string& resourceId)
{
    if (QString(resourceId.c_str()).startsWith("wms:"))
    {
        return true;//level < levelCount;
    }
    else
    {
        return QFileInfo(resourceId.c_str()).exists();
    }
}
