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

#ifndef _NUMBER_FORMAT_H_
#define _NUMBER_FORMAT_H_

#include <QString>

class NumberFormat
{
public:
    NumberFormat(unsigned int precision);

    QString toString(double value) const;

private:
    unsigned int m_precision;
};

#endif // _NUMBER_FORMAT_H_
