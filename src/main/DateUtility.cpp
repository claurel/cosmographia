// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
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

#include "DateUtility.h"

using namespace vesta;


QDateTime
VestaDateToQtDate(const GregorianDate& date)
{
    // Qt's DateTime class doesn't handle leap seconds
    int clampSec = std::min(int(date.second()), 59);

    return QDateTime(QDate(date.year(), date.month(), date.day()),
                     QTime(date.hour(), date.minute(), clampSec, date.usec() / 1000),
                     Qt::UTC);
}


GregorianDate
QtDateToVestaDate(const QDateTime& d)
{
    return GregorianDate(d.date().year(), d.date().month(), d.date().day(),
                         d.time().hour(), d.time().minute(), d.time().second(), d.time().msec() * 1000,
                         TimeScale_UTC);
}
