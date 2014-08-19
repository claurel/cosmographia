// This file is part of Cosmographia.
//
// Copyright (C) 2010-2012 Chris Laurel <claurel@gmail.com>
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

#include "NumberFormat.h"
#include <QLocale>
#include <cmath>

using namespace std;


NumberFormat::NumberFormat(unsigned int precision) :
    m_precision(precision)
{
}


static
QString readableNumber(double value, int significantDigits)
{
    double roundValue = value;
    int useDigits = 1;

    if (value != 0.0)
    {
        double n = log10(abs(value));
        useDigits = max(0, significantDigits - (int) n - 1);
        double m = pow(10.0, floor(n) - significantDigits + 1);
        roundValue = floor(value / m + 0.5) * m;
    }
    else
    {
        useDigits = significantDigits;
    }

    return QLocale::system().toString(roundValue, 'f', useDigits);
}


QString
NumberFormat::toString(double value) const
{
    return readableNumber(value, int(m_precision));
}
