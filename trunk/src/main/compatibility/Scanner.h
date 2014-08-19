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
