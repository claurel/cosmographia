// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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

#ifndef _NAME_TEMPLATE_TILED_MAP_H_
#define _NAME_PATTERN_TILED_MAP_H_

#include <vesta/HierarchicalTiledMap.h>
#include <vesta/TextureMapLoader.h>
#include <string>

class NameTemplateTiledMap : public vesta::HierarchicalTiledMap
{
public:
    NameTemplateTiledMap(vesta::TextureMapLoader* loader, const std::string& templ, unsigned int tileSize, unsigned int levelCount);
    ~NameTemplateTiledMap() {};
    
    virtual std::string tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row);
    virtual bool isValidTileAddress(unsigned int level, unsigned int column, unsigned int row);
    virtual bool tileResourceExists(const std::string& resourceId);

private:
    std::string m_nameTemplate;
    unsigned int m_levelCount;
};

#endif // _NAME_PATTERN_TILED_MAP_H_
