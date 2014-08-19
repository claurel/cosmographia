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
    m_hasError(false),
    m_topLevel(true)
{
    m_scanner = new Scanner(in);
}


CatalogParser::~CatalogParser()
{
    delete m_scanner;
}


/** Load an SSC (Solar System Catalog) object as a QVariant. An SSC object
  * has some properties that are specified outside of the table block. These
  * are:
  *    - the disposition (Add, Modify, Replace)
  *    - the object type (Body, AltSurface, or Location)
  *    - the object name
  *    - the parent object name
  *
  * These properties are stored in the QVariantMap as string properties with
  * the keys _disposition, _type, _name, and _parent.
  *
  * This method returns a QVariant with type Invalid if an object was not
  * read successfully. This can occur because of a parsing error or because
  * the end of file was reached.
  */
QVariant
CatalogParser::nextSscObject()
{
    QString disposition = "Add";
    QString type = "Body";

    QVariant v = nextValue();

    if (v.type() == QVariant::Invalid && atEnd())
    {
        // If we're at the end of the file, return
        // an empty value, but don't set an error.
        return QVariant();
    }

    // Parse the disposition (optional, defaults to "Add")
    if (v.type() == QVariant::ByteArray)
    {
        QString s = v.toString();
        if (s == "Add" || s == "Modify" || s == "Replace")
        {
            disposition = s;
            v = nextValue();
        }
    }

    // Parse the object type (optional, defaults to "Body")
    if (v.type() == QVariant::ByteArray)
    {
        QString s = v.toString();
        if (s == "Body" || s == "Location" || s == "AltSurface")
        {
            type = s;
            v = nextValue();
        }
    }

    // Parse the name (required)
    if (v.type() == QVariant::ByteArray)
    {
        if (!error())
        {
            setError(QString("Unknown object type %1").arg(v.toString()));
        }
        return QVariant();
    }
    else if (v.type() != QVariant::String)
    {
        if (!error())
        {
            setError("Object name expected in catalog file.");
        }
        return QVariant();
    }
    QString name = v.toString();

    // Parse the parent name (required)
    v = nextValue();
    if (v.type() != QVariant::String)
    {
        if (!error())
        {
            setError("Object parent name expected in catalog file.");
        }
        return QVariant();
    }
    QString parentName = v.toString();

    v = nextValue();
    if (v.type() != QVariant::Map)
    {
        if (!error())
        {
            setError("Object property table expected.");
        }
        return QVariant();
    }

    QVariantMap map = v.toMap();
    map["_disposition"] = disposition;
    map["_type"] = type;
    map["_name"] = name;
    map["_parent"] = parentName;

    return map;
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
QVariant
CatalogParser::nextValue()
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
    bool done = false;

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
    bool done = false;

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
            if (m_scanner->currentToken() == Scanner::Identifier)
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
