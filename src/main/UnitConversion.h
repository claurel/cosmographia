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

