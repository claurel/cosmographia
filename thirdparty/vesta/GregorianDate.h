/*
 * $Revision: 410 $ $Date: 2010-08-06 11:45:36 -0700 (Fri, 06 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_GREGORIAN_DATE_H_
#define _VESTA_GREGORIAN_DATE_H_

#include <iostream>


namespace vesta
{

class LeapSecondTable;

/** TimeScale
  */
enum TimeScale
{
    TimeScale_TDB  = 0, /**< Barycentric Dynamical Time */
    TimeScale_TT   = 1, /**< Terrestrial Time */
    TimeScale_TAI  = 2, /**< International Atomic Time */
    TimeScale_UTC  = 3, /**< Coordinated Universal Time */
};


/** An instance of GregorianDate the 'name' of an instant in time. The name can
  * be converted to a numeric value in one of VESTA's supported uniform time scales.
  * VESTA uses Barycentric Dynamical Time (TDB) as the time variable for planetary
  * ephemerides and spacecraft trajectories. Terrestrial Time (TT) and International
  * Atomic Time (TAI) are supported for conversions, but should not be used as inputs
  * for Trajectory and RotationModel classes. There's no conversion to a purely numeric
  * representation of UTC as the insertion of leap seconds means that it isn't a
  * uniform time scale.
  *
  * The most typical uses of GregorianDate are to convert from UTC to TDB and vice
  * versa. To convert a UTC calendar date to a time in seconds since J2000.0 TDB
  * (suitable for input to Trajectory::state()):
  *
  * \code
  * GregorianDate date(2010, 8, 28, 11, 30, 0);
  * double tdbsec = date.toTDBSec();
  * \endcode
  *
  * To create a human-readable date string from a time in seconds since J2000.0 TDB:
  * \code
  * string dateStr = GregorianDate::UTCDateFromTDBSec(tsec).toString();
  * \endcode
  */
class GregorianDate
{
public:
    enum Format
    {
        ISO8601_Combined = 0,
    };

    GregorianDate();
    GregorianDate(const GregorianDate& other);
    GregorianDate(int year, unsigned int month, unsigned int day,
                  unsigned int hour = 0, unsigned int minute = 0, unsigned int second = 0, unsigned int usec = 0,
                  TimeScale timeScale = TimeScale_UTC);

    GregorianDate& operator=(const GregorianDate& other);

    int year() const
    {
        return m_year;
    }

    unsigned int month() const
    {
        return m_month;
    }

    unsigned int day() const
    {
        return m_day;
    }

    unsigned int hour() const
    {
        return m_hour;
    }

    unsigned int minute() const
    {
        return m_minute;
    }

    unsigned int second() const
    {
        return m_second;
    }

    unsigned int usec() const
    {
        return m_usec;
    }

    TimeScale timeScale() const
    {
        return m_timeScale;
    }

    /** Change the time scale of this date. No conversion is applied, thus
      * calling this method with a different time scale means that the time
      * object will represent a different instant.
      */
    void setTimeScale(TimeScale timeScale)
    {
        m_timeScale = timeScale;
    }

    unsigned int dayOfWeek() const;
    unsigned int dayOfYear() const;
    unsigned int daysInMonth() const;
    int julianDay() const;

    bool isValid() const;

    bool isLeapYear() const;

    double toTDBJD() const;
    double toTAIJD() const;
    double toTTJD() const;
    double toTDBSec() const;
    double toTTSec() const;

    std::string toString(Format format = ISO8601_Combined) const;

    static GregorianDate UTCDateFromTDBJD(double tdbjd);
    static GregorianDate TDBDateFromTDBJD(double tdbjd);
    static GregorianDate UTCDateFromTDBSec(double tdbsec);
    static GregorianDate TDBDateFromTDBSec(double tdbsec);

private:
    int m_year;
    unsigned int m_month;
    unsigned int m_day;
    unsigned int m_hour;
    unsigned int m_minute;
    unsigned int m_second;
    unsigned int m_usec;
    TimeScale m_timeScale;

    static LeapSecondTable* s_DefaultLeapSecondTable;
};

}

std::ostream& operator<<(std::ostream& out, const vesta::GregorianDate& date);


#endif // _VESTA_GREGORIAN_DATE_H_
