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


