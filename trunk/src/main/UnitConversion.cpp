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

#include "UnitConversion.h"


static const double AU = 149597870.691;
static const double EarthMassKg = 5.9721986e24;

static MeasurementSystem s_DefaultMeasurementSystem = MetricUnits;

MeasurementSystem
GetDefaultMeasurementSystem()
{
    return s_DefaultMeasurementSystem;
}


void
SetDefaultMeasurementSystem(MeasurementSystem ms)
{
    s_DefaultMeasurementSystem = ms;
}


static
double timeUnitConversion(TimeUnit unit)
{
    switch (unit)
    {
    case Unit_Millisecond:
        return 0.001;
    case Unit_Second:
        return 1.0;
    case Unit_Minute:
        return 60.0;
    case Unit_Hour:
        return 3600.0;
    case Unit_Day:
        return 86400.0;
    case Unit_Year:
        return 365.25 * 86400.0;
    default:
        return 0.0;
    }
}


static
double distanceUnitConversion(DistanceUnit unit)
{
    switch (unit)
    {
    case Unit_Millimeter:
        return 1.0e-6;
    case Unit_Centimeter:
        return 1.0e-5;
    case Unit_Meter:
        return 1.0e-3;
    case Unit_Kilometer:
        return 1.0;
    case Unit_AU:
        return AU;
    case Unit_Foot:
        return 3.048e-4;
    case Unit_Yard:
        return 3 * 3.048e-4;
    case Unit_Mile:
        return 1.609344;
    default:
        return 0.0;
    }
}


static
double massUnitConversion(MassUnit unit)
{
    switch (unit)
    {
    case Unit_Gram:
        return 1.0e-3;
    case Unit_Kilogram:
        return 1.0;
    case Unit_EarthMass:
        return EarthMassKg;
    case Unit_MetricTon:
        return 1000.0;
    case Unit_Pound:
        return 0.45359237;
    case Unit_Ton:
        return 453.59237;
    default:
        return 0.0;
    }
}


double
ConvertTime(double value, TimeUnit fromUnit, TimeUnit toUnit)
{
    return value * timeUnitConversion(fromUnit) / timeUnitConversion(toUnit);
}


double
ConvertDistance(double value, DistanceUnit fromUnit, DistanceUnit toUnit)
{
    return value * distanceUnitConversion(fromUnit) / distanceUnitConversion(toUnit);
}


double
ConvertMass(double value, MassUnit fromUnit, MassUnit toUnit)
{
    return value * massUnitConversion(fromUnit) / massUnitConversion(toUnit);
}


