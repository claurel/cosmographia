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
