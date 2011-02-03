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

#include "CatalogParser.h"
#include "Scanner.h"
#include <cstdlib>
#include <cmath>

using namespace std;


// That this class emulates some Celestia functionality. Since the Celestia code is GPL
// instead of LGPL, this source module is a complete rewrite. No code from Celestia should
// be introduced here!


CatalogParser::CatalogParser(QIODevice* in) :
    m_scanner(NULL),
    m_topLevel(true)
{
    m_scanner = new Scanner(in);
}


CatalogParser::~CatalogParser()
{
    delete m_scanner;
}


/** Read the next value and store the result in a variant. The value will
  * have one of the following types:
  *    double
  *    integer
  *    string
  *    identifier
  *    boolean
  *    array
  *    table
  *
  * Catalog files distinguish between a string (a series of characters surrounded
  * by double quotes) and an identifier (a series of alphanumeric characters with
  * no quotes.) To distinguish these, we store strings into variants with type
  * QVariant::String and identifiers with type QVariant::ByteArray.
  *
  * Identifiers as values (rather than as table indexes) are only permitted at the
  * top level. This in order to support parsing the following syntax:
  *
  * Body "Earth" "Sol"
  * {
  *    ...
  * }
  *
  * Here, Body is actually a type tag, but parsing is simplified if we treat it as
  * a value.
  */
QVariant CatalogParser::nextValue()
{
    if (m_hasError)
    {
        return QVariant();
    }

    m_scanner->readNext();
    m_currentValue = readValue();

    return m_currentValue;
}


QVariant
CatalogParser::readValue()
{
    Scanner::TokenType token = m_scanner->currentToken();
    QVariant v;

    m_topLevel = true;

    switch (token)
    {
    case Scanner::Double:
        v = QVariant(m_scanner->doubleValue());
        break;

    case Scanner::Integer:
        v = QVariant(int(m_scanner->doubleValue()));
        break;

    case Scanner::String:
        v = QVariant(m_scanner->stringValue());
        break;

    case Scanner::Identifier:
        {
            QString id = m_scanner->stringValue();
            if (id == "true")
            {
                v = true;
            }
            else if (id == "false")
            {
                v = false;
            }
            else
            {
                if (m_topLevel)
                {
                    v = QVariant(id.toUtf8().data());
                }
                else
                {
                    setError("Identifier not expected at this point in catalog file.");
                }
            }
        }
        break;

    case Scanner::OpenBrace:
        v = readTable();
        break;

    case Scanner::OpenSquareBracket:
        v = readArray();
        break;

    case Scanner::EndToken:
        v = QVariant();
        break;

    default:
        if (m_scanner->error())
        {
            setError(m_scanner->errorMessage());
        }
        else
        {
            setError("Unexpected character in catalog file.");
        }
        break;
    }

    return v;
}


QVariant
CatalogParser::readArray()
{
    QVariantList array;
    bool ok = true;
    bool done = true;

    if (m_scanner->currentToken() != Scanner::OpenSquareBracket)
    {
        setError("Expected array in catalog file.");
        return QVariant();
    }

    m_topLevel = false;

    while (!done)
    {
        Scanner::TokenType token = m_scanner->readNext();
        if (token == Scanner::CloseSquareBracket)
        {
            done = true;
        }
        else
        {
            QVariant value = readValue();
            if (value.type() == QVariant::Invalid)
            {
                if (atEnd() && !m_scanner->error())
                {
                    setError("Unterminated array");
                }
                ok = false;
                done = true;
            }
            else
            {
                array << value;
            }
        }
    }

    if (!ok)
    {
        return QVariant();
    }
    else
    {
        return array;
    }
}


QVariant
CatalogParser::readTable()
{
    QVariantMap table;
    bool ok = true;
    bool done = true;

    if (m_scanner->currentToken() != Scanner::OpenBrace)
    {
        setError("Expected table in catalog file.");
        return QVariant();
    }

    m_topLevel = false;

    while (!done)
    {
        Scanner::TokenType token = m_scanner->readNext();
        if (token == Scanner::CloseBrace)
        {
            done = true;
        }
        else
        {
            QString key;
            if (m_scanner->readNext() == Scanner::Identifier)
            {
                key = m_scanner->stringValue();
            }
            else
            {
                setError("Identifier expected in table");
                return QVariant();
            }

            m_scanner->readNext();
            QVariant value = readValue();
            if (value.type() == QVariant::Invalid)
            {
                if (atEnd() && !m_scanner->error())
                {
                    setError("Unterminated table");
                }
                done = true;
                ok = false;
            }
            else
            {
                table[key] = value;
            }
        }
    }

    if (!ok)
    {
        return QVariant();
    }
    else
    {
        return table;
    }
}


void
CatalogParser::setError(const QString& errorMessage)
{
    m_hasError = true;
    m_errorMessage = errorMessage;
}


bool CatalogParser::error() const
{
    return m_scanner->error();
}


bool CatalogParser::atEnd() const
{
    return m_scanner->atEnd();
}
