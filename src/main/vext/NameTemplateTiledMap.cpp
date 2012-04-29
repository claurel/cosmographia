//
//  NameTemplateTiledMap.cpp
//  Cosmographia
//
//  Created by Chris Laurel on 4/2/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "NameTemplateTiledMap.h"
#include <QString>

using namespace vesta;
using namespace std;


NameTemplateTiledMap::NameTemplateTiledMap(TextureMapLoader* loader,
                                         const std::string& templ,
                                         unsigned int tileSize,
                                         unsigned int levelCount) :
    HierarchicalTiledMap(loader, tileSize),
    m_nameTemplate(templ),
    m_levelCount(levelCount)
{
}


string
NameTemplateTiledMap::tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row)
{
    // Tiles are arranged with north = 0
    unsigned int maxRow = (1u << level) - 1;
    
    /*
    ostringstream tileName;
    tileName << m_pattern << "_" << level << "_" << column << "_" << maxRow - row << ".pvr";

    return tileName.str();
    */

    // The external interface uses C++ strings, but for now use Qt to do the actual
    // substitution. The C++ standard library lacks the functions that would make it easy,
    // especially when dealing with UTF-8 strings.
    QString name = QString::fromUtf8(m_nameTemplate.c_str(), m_nameTemplate.length());
    name.replace("%level", QString::number(level));
    name.replace("%row", QString::number(maxRow - row));
    name.replace("%column", QString::number(column));

    return string(name.toUtf8().data());
}


bool 
NameTemplateTiledMap::isValidTileAddress(unsigned int level, unsigned int column, unsigned int row)
{
    unsigned int rowCount = 1u << level;
    unsigned int columnCount = 1u << (level + 1);
    return level < m_levelCount && row < rowCount && column < columnCount;
}


bool 
NameTemplateTiledMap::tileResourceExists(const std::string& /* resourceId */)
{
    return true;
}




