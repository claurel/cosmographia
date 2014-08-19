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

#ifndef _COMPATIBILITY_CATALOG_PARSER_H_
#define _COMPATIBILITY_CATALOG_PARSER_H_

#include <QIODevice>
#include <QVariant>

class Scanner;

class CatalogParser
{
public:
    CatalogParser(QIODevice* in);
    ~CatalogParser();

    QVariant nextValue();
    QVariant currentValue() const
    {
        return m_currentValue;
    }

    bool error() const;
    bool atEnd() const;
    QString errorMessage() const
    {
        return m_errorMessage;
    }

    QVariant nextSscObject();

private:
    QVariant readValue();
    QVariant readArray();
    QVariant readTable();
    void setError(const QString& errorMessage);

private:
    Scanner* m_scanner;
    QVariant m_currentValue;
    QString m_errorMessage;
    bool m_hasError;
    bool m_topLevel;
};

#endif // _COMPATIBILITY_CATALOG_PARSER_H_
