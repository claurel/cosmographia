// This file is part of Cosmographia.
//
// Copyright (C) 2010-2012 Chris Laurel <claurel@gmail.com>
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
