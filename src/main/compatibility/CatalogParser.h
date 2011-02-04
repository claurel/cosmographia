// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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
