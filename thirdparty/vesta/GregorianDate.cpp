/*
 * $Revision: 410 $ $Date: 2010-08-06 11:45:36 -0700 (Fri, 06 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "GregorianDate.h"
#include "Units.h"
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <limits>

using namespace vesta;
using namespace std;

static const unsigned int DaysPerMonth[] =
{
    0,
    31, 28, 31, 30, 31, 30,
    31, 31, 30, 31, 30, 31,
};


static const unsigned int DaysBeforeMonth[] =
{
    0,
      0,  31,  59,  90, 120, 151,
    181, 212, 243, 273, 304, 334,
};

// Difference in seconds between International Atomic Time (TAI) and
// Terrestrial Time (TT): TT = TAI + DeltaTAI
static const double DeltaTAI = 32.184;

// Constant values taken from SPICE leap second kernel naif0008.tls
static const double TDB_K  = 1.657e-3;
static const double TDB_EB = 1.671e-2;
static const double TDB_M0 = 6.239996;
static const double TDB_M1 = 1.99096871e-7;

// Convert from Terrestrial Time to Barycentric Dynamical Time. The argument and return
// value are both the number of seconds since J2000.0.
static double convertTTtoTDB(double ttSec)
{
    return ttSec + TDB_K * sin(TDB_M0 + TDB_M1 * ttSec + TDB_EB * sin(TDB_M0 + TDB_M1 * ttSec));
}

// Convert from Barycentric Dynamical Time to Terrestrial Time. The argument and return
// value are both the number of seconds since J2000.0.
static double convertTDBtoTT(double tdbSec)
{
    // We need to invert the expression in convertTTtoTDB. We'll approximate
    // a solution by iterating three times (which is what SPICE does). Note that
    // the maximum difference between the TT and TDB time scales is under two
    // milliseconds for any date within 1000 years of J2000.
    double ttSec = tdbSec;
    for (unsigned int i = 0; i < 3; ++i)
    {
        ttSec = tdbSec - TDB_K * sin(TDB_M0 + TDB_M1 * ttSec + TDB_EB * sin(TDB_M0 + TDB_M1 * ttSec));
    }

    return ttSec;
}

static double convertTAItoTT(double taiSec)
{
    return taiSec + DeltaTAI;
}

static double convertTTtoTAI(double ttSec)
{
    return ttSec - DeltaTAI;
}

static double convertTAItoTDB(double taiSec)
{
    return convertTTtoTDB(convertTAItoTT(taiSec));
}

static double convertTDBtoTAI(double tdbSec)
{
    return convertTTtoTAI(convertTDBtoTT(tdbSec));
}


// Convert a uniform time from a Julian Date to a count of seconds since
// J2000.0 (12:00:00 1-Jan-2000)
static double convertJDToSec(double jd)
{
    return (jd - 2451545.0) * 86400.0;
}

static double convertSecToJD(double sec)
{
    return sec / 86400.0 + 2451545.0;
}


/** Convert two times in seconds from one uniform time scale
  * to another.
  */
static double convertUniformSec(double fromTime, TimeScale fromScale, TimeScale toScale)
{
    if (fromScale == toScale)
    {
        return fromTime;
    }

    // Convert to TAI
    double tai = 0.0;
    switch (fromScale)
    {
    case TimeScale_TAI:
        tai = fromTime;
        break;

    case TimeScale_TDB:
        tai = convertTDBtoTAI(fromTime);
        break;

    case TimeScale_TT:
        tai = convertTTtoTAI(fromTime);
        break;

    default:
        assert(0);
    }

    switch (toScale)
    {
    case TimeScale_TAI:
        return tai;

    case TimeScale_TDB:
        return convertTAItoTDB(tai);

    case TimeScale_TT:
        return convertTAItoTT(tai);

    default:
        assert(0);
        return 0.0;
    }
}


/** Convert a Julian day number from one uniform time scale
  * to another.
  */
static double convertUniformJD(double fromTime, TimeScale fromScale, TimeScale toScale)
{
    return convertSecToJD(convertUniformSec(convertJDToSec(fromTime), fromScale, toScale));
}


// Get the Julian day number at noon on the specified Gregorian calendar date.
// If a date before 15 Oct 1582 is given, the Julian calendar is assumed. Conversion
// algorithm in from Meeus, _Astronomical Algorithms_.
static int julianDayNumber(int year, unsigned int month, unsigned int day)
{
    if (month <= 2)
    {
        year -= 1;
        month += 12;
    }

    int b;
    if (year > 1582 || (year == 1582 && (month > 10 || (month == 10 && day >= 15))))
    {
        b = 2 - (year / 100) + (year / 100) / 4;
    }
    else
    {
        // The specified day is in the before the Gregorian calendar
        // transition in October 1582.
        b = 0;
    }

    return int(365.25 * (year + 4716)) + int(30.6001 * (month + 1)) + day + b - 1524;
}


// Convert a uniform calendar date to a Julian day number in the same uniform time scale
static double UniformCalendarToJD(int year, unsigned int month, unsigned int day,
                                  unsigned int hour, unsigned int minute, double second)
{
    int dayNumber = julianDayNumber(year, month, day);

    // -0.5 is required because julianDayNumber returns the Julian day number at noon.
    return double(dayNumber) + (hour + (minute + second / 60.0) / 60.0) / 24.0 - 0.5;
}


// Convert day fraction to a time in hours, minutes, and seconds. This function handles leap seconds
// correctly: when fraction is >= 1, hours and minutes are clamped to 23 and 59, but second is allowed
// to be 60.
static void DayFractionToTime(double fracDay, unsigned int& hour, unsigned int& minute, double& second)
{
    double fracHour = fracDay * 24.0;
    hour = min(23, int(fracHour));

    double fracMinute = (fracHour - hour) * 60.0;
    minute = min(59, int(fracMinute));

    second = (fracMinute - minute) * 60.0;
}


static void JDToCalendar(double jd, int& year, unsigned int& month, unsigned int& day)
{
    int a = int(floor(jd + 0.5));

    double c;
    if (a < 2299161)
    {
        c = double(a + 1524);
    }
    else
    {
        double b = double(int(floor((a - 1867216.25) / 36524.25)));
        c = a + b - int(floor(b / 4)) + 1525;
    }

    int d = int(floor((c - 122.1) / 365.25));
    int e = int(floor(365.25 * d));
    int f = int(floor((c - e) / 30.6001));

    double fracDay = c - e - floor(30.6001 * f) + jd + 0.5 - a;

    month = f - 1 - 12 * (f / 14);
    year = d - 4715 - (7 + month) / 10;
    day = int(fracDay);
}


static void JDToCalendar(double jd,
                         int& year, unsigned int& month, unsigned int& day,
                         unsigned int& hour, unsigned int& minute, double& second)
{
    int a = int(floor(jd + 0.5));

    double c;
    if (a < 2299161)
    {
        c = double(a + 1524);
    }
    else
    {
        double b = double(int(floor((a - 1867216.25) / 36524.25)));
        c = a + b - int(floor(b / 4)) + 1525;
    }

    int d = int(floor((c - 122.1) / 365.25));
    int e = int(floor(365.25 * d));
    int f = int(floor((c - e) / 30.6001));

    double fracDay = c - e - floor(30.6001 * f) + jd + 0.5 - a;

    month = f - 1 - 12 * (f / 14);
    year = d - 4715 - (7 + month) / 10;
    day = int(fracDay);
    double fracHour = (fracDay - day) * 24.0;
    hour = int(fracHour);

    double fracMinute = (fracHour - hour) * 60.0;
    minute = int(fracMinute);

    second = (fracMinute - minute) * 60.0;
}


// Return true if the specified year is a leap year
static bool checkLeapYear(int y)
{
    return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
}


// Get the number of days in the specified month of the specified
// year.
static unsigned int daysInMonth(int y, int m)
{
    if (m == 2 && checkLeapYear(y))
    {
        return DaysPerMonth[m] + 1;
    }
    else
    {
        return DaysPerMonth[m];
    }
}


namespace vesta
{

struct LeapSecond
{
    int taiOffset;
    int year;
    unsigned int month;
    unsigned int day;
};

static const LeapSecond DefaultLeapSecondList[] =
{
    { 10, 1972,  1,  1 },
    { 11, 1972,  7,  1 },
    { 12, 1973,  1,  1 },
    { 13, 1974,  1,  1 },
    { 14, 1975,  1,  1 },
    { 15, 1976,  1,  1 },
    { 16, 1977,  1,  1 },
    { 17, 1978,  1,  1 },
    { 18, 1979,  1,  1 },
    { 19, 1980,  1,  1 },
    { 20, 1981,  7,  1 },
    { 21, 1982,  7,  1 },
    { 22, 1983,  7,  1 },
    { 23, 1985,  7,  1 },
    { 24, 1988,  1,  1 },
    { 25, 1990,  1,  1 },
    { 26, 1991,  1,  1 },
    { 27, 1992,  7,  1 },
    { 28, 1993,  7,  1 },
    { 29, 1994,  7,  1 },
    { 30, 1996,  1,  1 },
    { 31, 1997,  7,  1 },
    { 32, 1999,  1,  1 },
    { 33, 2006,  1,  1 },
    { 34, 2009,  1,  1 },
    { 35, 2012,  7,  1 },
};


// Get a unique value for the day that can be used to lookup in the leap
// second table.
static unsigned int dateHash(int year, unsigned int month, unsigned int day)
{
    return day + 100 * (month + 100 * year);
}


struct UTCDifferenceRecord
{
    double tai;
    double diffSec;
};


bool operator<(const UTCDifferenceRecord& t0, const UTCDifferenceRecord& t1)
{
    return t0.tai < t1.tai;
}


bool operator<(const LeapSecond& ls0, const LeapSecond& ls1)
{
    if (ls0.year < ls1.year)
    {
        return true;
    }
    else if (ls0.year > ls1.year)
    {
        return false;
    }
    else
    {
        if (ls0.month < ls1.month)
        {
            return true;
        }
        else if (ls0.month > ls1.month)
        {
            return false;
        }
        else
        {
            return ls0.day < ls1.day;
        }
    }
}


/** LeapSecondTable is an internal class used to calculate the difference
  * between UTC and TAI at some instant in time. This class will eventually
  * be exposed so that VESTA users can install custom leap second tables.
  */
class LeapSecondTable
{
public:
    LeapSecondTable(const LeapSecond leapSeconds[], unsigned int leapSecondCount)
    {
        for (unsigned int i = 0; i < leapSecondCount; ++i)
        {
            const LeapSecond& ls = leapSeconds[i];
            m_leapSeconds.push_back(ls);
            m_calendarOffsets[dateHash(ls.year, ls.month, ls.day)] = ls.taiOffset;

            UTCDifferenceRecord utcDiff;
            utcDiff.diffSec = ls.taiOffset;
            utcDiff.tai = UniformCalendarToJD(ls.year, ls.month, ls.day, 0, 0, 0) + secondsToDays(ls.taiOffset);
            m_utcDiffs.push_back(utcDiff);
       }
   }

   bool dateHasLeapSecond(const GregorianDate& d)
   {
       // The leap year offset table stores days *after* the ones
       // containing leap seconds. Advance one day to check the table
       // for a leap second...
       unsigned int year = d.year();
       unsigned int month = d.month();
       unsigned int day = d.day();

       if (d.day() == daysInMonth(year, month))
       {
           if (month == 12)
           {
               month = 1;
               year++;
           }
           else
           {
               month++;
           }

           day = 1;
       }
       else
       {
           day++;
       }

       return m_calendarOffsets.find(dateHash(year, month, day)) != m_calendarOffsets.end();
   }


   // Because of leap seconds, UTC is not a uniform time system: it is split into uniform
   // intervals that begin and end when leap seconds are added (or subtracted, though
   // to date there has not been a negative leap second.) This function converts atomic
   // time (TAI) to a UTC Julian day number and day fraction.
   void taiToUTCDayAndFraction(double taijd, double* utcDay, double* dayFraction)
   {
       // Empty leap seconds table
       if (m_utcDiffs.empty())
       {
           double utc = taijd;
           *utcDay = floor(utc + 0.5) - 0.5;
           *dayFraction = utc - *utcDay;
           return;
       }

       // Find the uniform interval containing the TAI instant
       UTCDifferenceRecord comp;
       comp.tai = taijd;
       comp.diffSec = 0.0;
       vector<UTCDifferenceRecord>::const_iterator iter = lower_bound(m_utcDiffs.begin(), m_utcDiffs.end(), comp);

       // The instant occurs before the introduction of leap seconds
       if (iter == m_utcDiffs.begin())
       {
           double utc = taijd - secondsToDays(iter->diffSec);
           *utcDay = floor(utc + 0.5) - 0.5;
           *dayFraction = utc - *utcDay;
           return;
       }

       UTCDifferenceRecord timeInterval;
       double intervalLength;
       if (iter == m_utcDiffs.end())
       {
           // The instant occurs in the last time interval (need special handling because end() - 1 isn't valid)
           timeInterval = m_utcDiffs.back();
           intervalLength = numeric_limits<double>::infinity();
       }
       else
       {
           timeInterval = *(iter - 1);
           intervalLength = iter->tai - timeInterval.tai;
       }

       double utcBase = timeInterval.tai - secondsToDays(timeInterval.diffSec);
       double utcOffset = taijd - timeInterval.tai;

       double days = floor(utcOffset);
       *utcDay = utcBase + days;
       *dayFraction = utcOffset - days;
       if (intervalLength < numeric_limits<double>::infinity())
       {
           if (intervalLength - days < 0.5)
           {
               // We're in a day containing a leap second; decrement the day count and increment the day fraction
               *utcDay -= 1.0;
               *dayFraction += 1.0;
           }
       }
   }


   // Get the difference betweeen UTC and TAI at the specified UTC
   // calendar day.
   double utcDifference(int year, unsigned int month, unsigned int day)
   {
       if (m_utcDiffs.empty())
       {
           return 0.0;
       }

       LeapSecond ls;
       ls.year = year;
       ls.month = month;
       ls.day = day;
       vector<LeapSecond>::const_iterator iter = lower_bound(m_leapSeconds.begin(), m_leapSeconds.end(), ls);

       if (iter == m_leapSeconds.end())
       {
           return m_leapSeconds.back().taiOffset;
       }
       else if (ls < *iter)
       {
           if (iter == m_leapSeconds.begin())
           {
               return iter->taiOffset;
           }
           else
           {
               --iter;
               return iter->taiOffset;
           }
       }
       else
       {
           return iter->taiOffset;
       }
   }

private:
    vector<LeapSecond> m_leapSeconds;
    map<unsigned int, unsigned int> m_calendarOffsets;
    vector<UTCDifferenceRecord> m_utcDiffs;
};

}


vesta::LeapSecondTable* GregorianDate::s_DefaultLeapSecondTable =
        new vesta::LeapSecondTable(DefaultLeapSecondList,
                                   sizeof(DefaultLeapSecondList) / sizeof(DefaultLeapSecondList[0]));


/** Default constructor creates a date representing the instant at midnight, 1 January 2000 UTC.
  */
GregorianDate::GregorianDate() :
    m_year(2000),
    m_month(1),
    m_day(1),
    m_hour(0),
    m_minute(0),
    m_second(0),
    m_usec(0),
    m_timeScale(TimeScale_UTC)
{
}


/** Copy constructor.
  */
GregorianDate::GregorianDate(const GregorianDate& other) :
    m_year(other.m_year),
    m_month(other.m_month),
    m_day(other.m_day),
    m_hour(other.m_hour),
    m_minute(other.m_minute),
    m_second(other.m_second),
    m_usec(other.m_usec),
    m_timeScale(other.m_timeScale)
{
}


/** Construct a new calendar date.
  *
  * The time of day and time scale are optional. If they are omitted, the time is set to midnight
  * and the time scale to UTC.
  *
  * \param year astronomical year number (for year < 1, year 0 = 1 BCE, year -1 = 2 BCE, etc.)
  * \param month month number (1 - 12)
  * \param day day number (1 - 31)
  * \param hour hour number (0 - 23)
  * \param minute minute number (0 - 59)
  * \param second second number (0 - 59, 60 allowed for leap seconds in UTC time scale)
  * \param usec microseconds (0 - 999999)
  */
GregorianDate::GregorianDate(int year, unsigned int month, unsigned int day,
                             unsigned int hour, unsigned int minute, unsigned int second, unsigned int usec,
                             TimeScale timeScale) :
    m_year(year),
    m_month(month),
    m_day(day),
    m_hour(hour),
    m_minute(minute),
    m_second(second),
    m_usec(usec),
    m_timeScale(timeScale)
{
}


/** Assignment operator.
  */
GregorianDate& GregorianDate::operator=(const GregorianDate& other)
{
    m_year = other.m_year;
    m_month = other.m_month;
    m_day = other.m_day;
    m_hour = other.m_hour;
    m_minute = other.m_minute;
    m_second = other.m_second;
    m_usec = other.m_usec;
    m_timeScale = other.m_timeScale;

    return *this;
}


/** Return true if this calendar date names a real instant in time.
  */
bool
GregorianDate::isValid() const
{
    unsigned int monthLength = m_month <= 12 ? DaysPerMonth[m_month] : 0;
    if (isLeapYear() && m_month == 2)
    {
        monthLength++;
    }

    unsigned int minuteLength = 60;
    if (s_DefaultLeapSecondTable->dateHasLeapSecond(*this) && m_timeScale == TimeScale_UTC)
    {
        // TODO: Handle negative leap seconds so that the code doesn't break in the
        // event that one is ever added.
        minuteLength = 61;
    }

    if (m_year == 1582 && m_month == 10 && m_day > 4 && m_day < 15)
    {
        // Skipped days during the Julian to Gregorian calendar
        // transition.
        return false;
    }
    else
    {
        return (m_month > 0 && m_month <= 12) &&
               (m_day > 0 && m_day <= monthLength) &&
               m_hour < 24 &&
               m_minute < 60 &&
               m_second < minuteLength &&
               m_usec < 1000000;
    }
}


/** Return true if the date falls within a leap year.
  */
bool
GregorianDate::isLeapYear() const
{
    return checkLeapYear(m_year);
}


/** Return the day number within the year. This will be a value
  * from 1 -- 365 (or 366 during a leap year.)
  */
unsigned int
GregorianDate::dayOfYear() const
{
    unsigned int daysBefore = DaysBeforeMonth[m_month];
    if (m_month > 2 && isLeapYear())
    {
        ++daysBefore;
    }

    return daysBefore + m_day;
}


/** Returns the day of the week as an integer between 1 and 7.
  */
unsigned int
GregorianDate::dayOfWeek() const
{
    return (julianDay() + 1) % 7 + 1;
}


/** Return the number of days in the month: 28, 29, 30, or 31 depending
  * on the month and whether the year is a leap year.
  */
unsigned int
GregorianDate::daysInMonth() const
{
    return ::daysInMonth(m_year, m_month);
}


/** Get the Julian day number (days since 1 Nov 4713 BCE) of this
  * date.
  */
int
GregorianDate::julianDay() const
{
    return julianDayNumber(m_year, m_month, m_day);
}


/** Convert the date to a Julian day number in the TDB time scale.
  */
double
GregorianDate::toTDBJD() const
{
    return convertSecToJD(toTDBSec());
}


/** Convert the date to a Julian day number in the TAI time scale.
  */
double
GregorianDate::toTAIJD() const
{
    double second = m_second + m_usec * 1.0e-6;

    double uniformTime = UniformCalendarToJD(m_year, m_month, m_day, m_hour, m_minute, second);
    TimeScale timeScale = m_timeScale;
    if (m_timeScale == TimeScale_UTC)
    {
        double utcOffset = s_DefaultLeapSecondTable->utcDifference(m_year, m_month, m_day);
        uniformTime += secondsToDays(utcOffset);
        timeScale = TimeScale_TAI;
    }

    return convertUniformJD(uniformTime, timeScale, TimeScale_TAI);
}


/** Convert the date to a Julian day number in the TT time scale.
  */
double
GregorianDate::toTTJD() const
{
    return toTAIJD() + secondsToDays(DeltaTAI);
}


/** Convert the date to a number of seconds since J2000.0 in the TDB
  * (Barycentric Dynamical Time) time scale.
  */
double
GregorianDate::toTDBSec() const
{
    return convertTAItoTDB(convertJDToSec(toTAIJD()));
}


/** Convert the date to a number of seconds since J2000.0 in the TT
  * (Terrestrial Time) time scale.
  */
double
GregorianDate::toTTSec() const
{
    return convertTAItoTT(convertJDToSec(toTAIJD()));
}


/** Construct a UTC calendar date from a Julian day number in the TDB time scale.
  */
GregorianDate
GregorianDate::UTCDateFromTDBJD(double tdbjd)
{
    int year = 0;
    unsigned int month = 1;
    unsigned int day = 1;
    unsigned int hour = 0;
    unsigned int minute = 0;
    double second = 0;

    double tai = tdbjd - DeltaTAI / 86400.0;    

    // Convert TAI to a UTC day and day fraction. If the instant occurs during a
    // leap second, dayFraction will be >= 1
    double utcDay = 0.0;
    double dayFraction = 0.0;
    s_DefaultLeapSecondTable->taiToUTCDayAndFraction(tai, &utcDay, &dayFraction);

    // Get the calendar day; add a small fraction to prevent rounding errors
    JDToCalendar(utcDay + 0.01, year, month, day);

    // Convert the day fraction to a time of day
    DayFractionToTime(dayFraction, hour, minute, second);
    unsigned int s = unsigned(second);
    unsigned int usec = unsigned((second - s) * 1.0e6);

    return GregorianDate(year, month, day, hour, minute, s, usec, TimeScale_UTC);
}


/** Construct a TDB calendar date from a Julian day number in the TDB time scale.
  */
GregorianDate
GregorianDate::TDBDateFromTDBJD(double tdbjd)
{
    int year = 0;
    unsigned int month = 1;
    unsigned int day = 1;
    unsigned int hour = 0;
    unsigned int minute = 0;
    double second = 0;

    JDToCalendar(tdbjd, year, month, day, hour, minute, second);
    unsigned int s = unsigned(second);
    unsigned int usec = unsigned((second - s) * 1.0e6);

    return GregorianDate(year, month, day, hour, minute, s, usec, TimeScale_TDB);
}


GregorianDate
GregorianDate::UTCDateFromTDBSec(double tdbsec)
{
    return UTCDateFromTDBJD(convertSecToJD(tdbsec));
}


/** Construct a TDB calendar date from a Julian day number in the TDB time scale.
  */
GregorianDate
GregorianDate::TDBDateFromTDBSec(double tdbsec)
{
    return TDBDateFromTDBJD(convertSecToJD(tdbsec));
}


/** Convert the date to a string with the specified format.
  */
string
GregorianDate::toString(Format format) const
{
    if (format == ISO8601_Combined)
    {
        ostringstream str;
        str << year() << '-'
            << setw(2) << setfill('0') << month() << '-'
            << setw(2) << setfill('0') << day() << 'T'
            << setw(2) << setfill('0') << hour() << ':'
            << setw(2) << setfill('0') << minute() << ':'
            << setw(2) << setfill('0') << second();

        switch (m_timeScale)
        {
        case TimeScale_TDB:
            str << " TDB";
            break;
        case TimeScale_TT:
            str << " TT";
            break;
        case TimeScale_TAI:
            str << " TAI";
            break;
        case TimeScale_UTC:
            str << " UTC";
            break;
        }

        return str.str();
    }
    else
    {
        return "";
    }
}

std::ostream& operator<<(std::ostream& out, const vesta::GregorianDate& date)
{
    out << date.toString();
    return out;
}
