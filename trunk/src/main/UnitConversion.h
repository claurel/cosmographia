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

#ifndef _UNIT_CONVERSION_H_
#define _UNIT_CONVERSION_H_

enum MeasurementSystem
{
    MetricUnits,
    ImperialUnits
};

enum TimeUnit
{
    Unit_Millisecond,
    Unit_Second,
    Unit_Minute,
    Unit_Hour,
    Unit_Day,
    Unit_Year,
    InvalidTimeUnit
};

enum DistanceUnit
{
    Unit_Millimeter,
    Unit_Centimeter,
    Unit_Meter,
    Unit_Kilometer,
    Unit_AU,
    Unit_Foot,
    Unit_Yard,
    Unit_Mile,
    InvalidDistanceUnit
};

enum MassUnit
{
    Unit_Kilogram,
    Unit_Gram,
    Unit_EarthMass,
    Unit_MetricTon,
    Unit_Pound,
    Unit_Ton,
    InvalidMassUnit
};

double ConvertTime(double value, TimeUnit fromUnit, TimeUnit toUnit);
double ConvertDistance(double value, DistanceUnit fromUnit, DistanceUnit toUnit);
double ConvertMass(double value, MassUnit fromUnit, MassUnit toUnit);

MeasurementSystem GetDefaultMeasurementSystem();
void SetDefaultMeasurementSystem(MeasurementSystem ms);

#endif // _UNIT_CONVERSION_H_

