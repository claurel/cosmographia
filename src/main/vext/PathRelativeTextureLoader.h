// This file is part of Cosmographia.
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

#ifndef _PATH_RELATIVE_TEXTURE_LOADER_H_
#define _PATH_RELATIVE_TEXTURE_LOADER_H_

#include <vesta/TextureMapLoader.h>

class PathRelativeTextureLoader : public vesta::TextureMapLoader
{
public:
    PathRelativeTextureLoader();
    virtual ~PathRelativeTextureLoader();

    virtual std::string searchPath() const = 0;
    virtual void setSearchPath(const std::string& path) = 0;

    virtual bool handleMakeResident(vesta::TextureMap* texture) = 0;

protected:
    virtual std::string resolveResourceName(const std::string& resourceName) = 0;
};

#endif // _PATH_RELATIVE_TEXTURE_LOADER_H_
