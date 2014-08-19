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

#include "Scanner.h"
#include <cstdlib>
#include <cstdio>
#include <cmath>

using namespace std;


// That this class emulates some Celestia functionality. Since the Celestia code is GPL
// instead of LGPL, this source module is a complete rewrite. No code from Celestia should
// be introduced here!

static const int EndOfFile = EOF;


/** The Scanner class is intended to be used for parsing tokens in Celestia
  * text catalog files (SSC, STC, DSC).
  */
Scanner::Scanner(QIODevice *in) :
    m_in(in),
    m_currentTokenType(NoToken),
    m_skipRead(false),
    m_nextChar(' '),
    m_doubleValue(0.0)
{
}


Scanner::~Scanner()
{
}


enum ScannerState
{
    BeginTokenState,
    EndTokenState,
    IntegerState,
    ExponentSignState,
    ExponentState,
    FractionState,
    IdentifierState,
    CommentState,
    StringState,
    StringEscapeState,
};


static int digitValue(int c)
{
    return int(c) - int('0');
}

static bool isIdentifierCharacter(int c)
{
    return isalpha(c) || isdigit(c) || c == '_';
}

static bool isTokenSeparator(int c)
{
    return !isIdentifierCharacter(c) && c != '.';
}


/** Read the next token and return its type.
  *
  * Once an error is reported by readNext(), no subsequent reads will succeed.
  */
Scanner::TokenType
Scanner::readNext()
{
    ScannerState state = BeginTokenState;

    double exponentValue = 0.0;
    double integerValue = 0.0;
    double fractionValue = 0.0;
    int fractionLength = 0;
    int numberSign = 1;
    int exponentSign = 1;

    m_stringValue = "";
    m_doubleValue = 0.0;

    // Once an error has occurred, always report failure.
    if (m_currentTokenType == Invalid)
    {
        return m_currentTokenType;
    }
    else if (m_nextChar == EndOfFile)
    {
        m_currentTokenType = EndToken;
        return m_currentTokenType;
    }

    while (state != EndTokenState)
    {
        if (m_skipRead)
        {
            m_skipRead = false;
        }
        else
        {
            char c = 0;
            bool ok = m_in->getChar(&c);
            m_nextChar = c;

            if (!ok)
            {
                m_nextChar = EndOfFile;
                if (!m_in->atEnd())
                {
                    setErrorState("Error reading stream.");
                    state = EndTokenState;
                }
            }
        }

        switch (state)
        {
        case BeginTokenState:
            if (m_nextChar == EndOfFile)
            {
                state = EndTokenState;
                m_currentTokenType = EndToken;
            }
            else if (isspace(m_nextChar))
            {
                // Nothing
            }
            else if (isdigit(m_nextChar))
            {
                state = IntegerState;
                integerValue = digitValue(m_nextChar);
            }
            else if (m_nextChar == '-')
            {
                state = IntegerState;
                numberSign = -1;
                integerValue = 0;
            }
            else if (m_nextChar == '+')
            {
                state = IntegerState;
                integerValue = 0;
            }
            else if (m_nextChar == '.')
            {
                state = FractionState;
            }
            else if (isalpha(m_nextChar) || m_nextChar == '_')
            {
                state = IdentifierState;
                m_stringValue += (char) m_nextChar;
            }
            else if (m_nextChar == '#')
            {
                state = CommentState;
            }
            else if (m_nextChar == '"')
            {
                state = StringState;
            }
            else if (m_nextChar == '{')
            {
                state = EndTokenState;
                m_currentTokenType = OpenBrace;
            }
            else if (m_nextChar == '}')
            {
                state = EndTokenState;
                m_currentTokenType = CloseBrace;
            }
            else if (m_nextChar == '[')
            {
                state = EndTokenState;
                m_currentTokenType = OpenSquareBracket;
            }
            else if (m_nextChar == ']')
            {
                state = EndTokenState;
                m_currentTokenType = CloseSquareBracket;
            }
            else
            {
                state = EndTokenState;
                setErrorState(QString("Invalid character '%1' in stream").arg(m_nextChar));
            }
            break;

        case IdentifierState:
            if (isIdentifierCharacter(m_nextChar))
            {
                m_stringValue += m_nextChar;
            }
            else
            {
                state = EndTokenState;
                m_currentTokenType = Identifier;
            }
            break;

        case IntegerState:
            if (isdigit(m_nextChar))
            {
                integerValue = integerValue * 10.0 + digitValue(m_nextChar);
            }
            else if (m_nextChar == '.')
            {
                state = FractionState;
            }
            else if (m_nextChar == 'e' || m_nextChar == 'E')
            {
                state = ExponentSignState;
            }
            else if (isTokenSeparator(m_nextChar))
            {
                state = EndTokenState;
                m_currentTokenType = Integer;
                m_skipRead = true;
            }
            else
            {
                state = EndTokenState;
                setErrorState("Invalid character in number");
            }
            break;

        case FractionState:
            if (isdigit(m_nextChar))
            {
                fractionValue = fractionValue * 10.0 + digitValue(m_nextChar);
                fractionLength++;
            }
            else if (m_nextChar == 'e' || m_nextChar == 'E')
            {
                state = ExponentSignState;
            }
            else if (isTokenSeparator(m_nextChar))
            {
                state = EndTokenState;
                m_currentTokenType = Double;
                m_skipRead = true;
            }
            else
            {
                state = EndTokenState;
                setErrorState("Invalid character in number");
            }
            break;

        case ExponentSignState:
            if (m_nextChar == '-')
            {
                state = ExponentState;
                exponentSign = -1;
            }
            else if (m_nextChar == '+')
            {
                state = ExponentState;
            }
            else if (isdigit(m_nextChar))
            {
                state = ExponentState;
                exponentValue += ((int) m_nextChar - (int) '0');
            }
            else if (isTokenSeparator(m_nextChar))
            {
                state = EndTokenState;
                m_currentTokenType = Double;
                m_skipRead = true;
            }
            else
            {
                state = EndTokenState;
                setErrorState("Invalid character in exponent");
            }
            break;

        case ExponentState:
            if (isdigit(m_nextChar))
            {
                exponentValue = exponentValue * 10.0 + digitValue(m_nextChar);
            }
            else if (isTokenSeparator(m_nextChar))
            {
                state = EndTokenState;
                m_currentTokenType = Double;
                m_skipRead = true;
            }
            else
            {
                state = EndTokenState;
                setErrorState("Invalid character in exponent");
            }
            break;

        case CommentState:
            if (m_nextChar == '\n' || m_nextChar == '\r' || m_nextChar == EndOfFile)
            {
                state = BeginTokenState;
            }
            break;

        case StringState:
            if (m_nextChar == '"')
            {
                // Finished the string
                state = EndTokenState;
                m_currentTokenType = String;
            }
            else if (m_nextChar == '\\')
            {
                state = StringEscapeState;
            }
            else if (m_nextChar == EndOfFile)
            {
                setErrorState("Unterminated string");
            }
            else
            {
                // Add another character to the string
                m_stringValue += m_nextChar;
            }
            break;

        case StringEscapeState:
            if (m_nextChar == 'n')
            {
                m_stringValue += '\n';
                state = StringState;
            }
            else if (m_nextChar == 't')
            {
                m_stringValue += '\n';
                state = StringState;
            }
            else if (m_nextChar == '\\')
            {
                m_stringValue += '\\';
                state = StringState;
            }
            else if (m_nextChar == '"')
            {
                m_stringValue += '"';
                state = StringState;
            }
            else
            {
                setErrorState(QString("Invalid string escape \\%1").arg((char) m_nextChar));
                state = EndTokenState;
            }
            break;

        case EndTokenState:
            break;
        }
    }

    if (m_currentTokenType == Integer)
    {
        m_doubleValue = integerValue * numberSign;
    }
    else if (m_currentTokenType == Double)
    {
        double mantissa = numberSign * (integerValue + fractionValue * pow(10.0, double(-fractionLength)));
        m_doubleValue = mantissa * pow(10.0, double(exponentValue * exponentSign));
    }

    return m_currentTokenType;
}


void
Scanner::setErrorState(const QString &message)
{
    m_errorMessage = message;
    m_currentTokenType = Invalid;
}
