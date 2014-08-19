// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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

#include "Addon.h"


AddOn::AddOn()
{
}


AddOn::~AddOn()
{
}


void
AddOn::setSource(const QString& source)
{
    m_source = source;
}


void
AddOn::setTitle(const QString& title)
{
    m_title = title;
}


void
AddOn::addObject(const QString& objectName)
{
    m_objects << objectName;
}
