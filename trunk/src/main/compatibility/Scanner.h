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

#ifndef _COMPATIBILITY_SCANNER_H_
#define _COMPATIBILITY_SCANNER_H_

#include <QIODevice>


class Scanner
{
public:
    Scanner(QIODevice* in);
    ~Scanner();

    enum TokenType
    {
        NoToken,
        EndToken,
        Invalid,
        Identifier,
        String,
        Double,
        Integer,
        OpenBrace,
        CloseBrace,
        OpenSquareBracket,
        CloseSquareBracket,
    };

    TokenType readNext();

    TokenType currentToken() const
    {
        return m_currentTokenType;
    }

    QString stringValue() const
    {
        return m_stringValue;
    }

    double doubleValue() const
    {
        return m_doubleValue;
    }

    QString errorMessage() const
    {
        return m_errorMessage;
    }

    bool error() const
    {
        return m_currentTokenType == Invalid;
    }

    bool atEnd() const
    {
        return m_currentTokenType == EndToken;
    }

private:
    void setErrorState(const QString& message);

private:
    QIODevice* m_in;
    TokenType m_currentTokenType;
    QString m_errorMessage;

    bool m_skipRead;
    int m_nextChar;

    double m_doubleValue;
    QString m_stringValue;
};

#endif // _COMPATIBILITY_SCANNER_H_
