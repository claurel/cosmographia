// -----------------------------------------------------------------------------------------------
//
// This file is part of a free package distributed by Dr TS Kelso, tkelso@grove.net in his web site
// at http://www.grove.net/~tkelso/
//
// This system carries two-line orbital data (*.tle files with the master list being tle.new) 
// and the following key programs:
//
//* spacetrk.zip (LaTeX documentation and FORTRAN source code for the NORAD
//				orbital models needed for using the two-line element sets),
//* sat-db13.zip (a complete database of all payloads launched into orbit),
//* sgp4-pl2.zip (a library of Turbo Pascal units to implement SGP4/SDP4),
//* trakstr2.zip (an implementation of sgp4-pl2 to output satellite ECI
//				position, subpoint, look angles, and right ascension/declination),
//* passupdt.zip (a package for quickly and easily updating two-line element
//				sets), and
//* unzip (a Sun executable for unzipping these programs).
//
// More information on the first five files is available in the file master.dir.
//
//
//- Dr TS Kelso, tkelso@grove.net
//	Adjunct Professor of Space Operations
//	Air Force Institute of Technology
//	SYSOP, Celestial WWW
//				       http://www.grove.net/~tkelso/
//
// -----------------------------------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include "norad.h"

#define PI 3.141592653589793238462643383279502884197
#define TWOPI (2. * PI)
#define MINUTES_PER_DAY 1440.
#define MINUTES_PER_DAY_SQUARED (MINUTES_PER_DAY * MINUTES_PER_DAY)
#define MINUTES_PER_DAY_CUBED (MINUTES_PER_DAY * MINUTES_PER_DAY_SQUARED)
#define AE 1.0
                             /* distance units, earth radii */

/* Converts the quasi scientific notation of the "Motion Dot Dot/6" or
"BSTAR" field to double. */

static double sci( const char *string)
{
   char buf[12], *bufptr = buf;

   if( string[1] == ' ')   /* is field blank? */
      return 0.;

   /* get mantissa */
   if (*string == '-')
      *bufptr++ = '-';
   ++string;      /* point to 1st digit of mantissa */
   *bufptr++ = '.';
   strncpy(bufptr, string, 6);   /* mantissa has 5 digits */
   bufptr += 5;
   string += 5;
   *bufptr++ = 'E';

   /* get exponent */
   if (*string == '-')
      *bufptr++ = '-';   /* exponent sign, if neg. */

   *bufptr++ = *++string;      /* copy exponent */
   *bufptr = '\0';
   return( atof(buf));
}

/* Does a checksum modulo 10 on the given line.  Digits = their
value, '-' = 1, all other chars = 0.  Returns 0 if ok, a negative
value if it's definitely not a TLE line,  positive if it's all OK
except the checksum.  This last was added because people sometimes
want to use TLEs without worrying about the checksum. */

int DLL_FUNC tle_checksum( const char *buff)
{
   int rval = 0;
   int count = 69;

   if( *buff != '1' && *buff != '2' || buff[1] != ' ')
      return( -1);
   while( --count)
      {
      if( *buff > '0' && *buff <= '9')
         rval += *buff - '0';
      else if( *buff == '-')
         rval++;
      if( *buff < ' ' || *buff > 'z')           /* invalid character */
         return( -2);
      buff++;
      }
   rval -= *buff++ - '0';
   if( *buff > ' ')                 /* line unterminated */
      rval = -3;
   else
      {
      rval %= 10;
      if( rval < 0)
         rval += 10;
      }
   return( rval % 10);
}

/* Meteor 2-08                                                           */
/* 1 13113U          88245.60005115 0.00000076           63463-4 0  5998 */
/* 2 13113  82.5386 288.0994 0015973 147.1294 213.0868 13.83869004325321 */

#define J1900 (2451545.5 - 36525. - 1.)

/* parse_elements returns:
         0 if the elements are parsed without error;
         1 if they're OK except the first line has a checksum error;
         2 if they're OK except the second line has a checksum error;
         3 if they're OK except both lines have checksum errors;
         a negative value if the lines aren't at all parseable */

int DLL_FUNC parse_elements( const char *line1, const char *line2, tle_t *sat)
{
   int year, rval, checksum_problem = 0;

   if( *line1 != '1' || *line2 != '2')
      rval = -4;
   else
      {
      rval = tle_checksum( line1);
      if( rval > 0)
         {
         checksum_problem = 1;  /* there's a checksum problem,  but it's */
         rval = 0;              /* not fatal; continue processing the TLE */
         }
      }

   if( rval)
      rval -= 100;
   else
      {
      rval = tle_checksum( line2);
      if( rval > 0)
         {
         checksum_problem |= 2;  /* there's a checksum problem,  but it's */
         rval = 0;               /* not fatal; continue processing the TLE */
         }
      }

   if( !rval)
      {
      char tbuff[12];

      sat->xmo = (PI / 180.) * atof( line2 + 43);
      sat->xnodeo = (PI / 180.) * atof( line2 + 17);
      sat->omegao = (PI / 180.) * atof( line2 + 34);
      sat->xincl = (PI / 180.) * atof( line2 + 8);

      /* put a decimal point in front of eccentricity, decode it */
      tbuff[11] = '\0';
      memcpy( tbuff, line2 + 25, 11);
      *tbuff = '.';
      sat->eo = atof( tbuff);

      /* Make sure mean motion is null-terminated, since rev. no.
      may immediately follow. */
      memcpy( tbuff, line2 + 52, 11);
            /* Input mean motion,  derivative of mean motion and second  */
            /* deriv of mean motion,  are all in revolutions and days.   */
            /* Convert them here to radians and minutes:                 */
      sat->xno = atof( tbuff) * TWOPI / MINUTES_PER_DAY;
      sat->xndt2o = atof( line1 + 33) * TWOPI / MINUTES_PER_DAY_SQUARED;
      sat->xndd6o = sci( line1 + 44) * TWOPI / MINUTES_PER_DAY_CUBED;

      sat->bstar = sci(line1 + 53) * AE;
      year = line1[19] - '0';
      if( line1[18] >= '0')
         year += (line1[18] - '0') * 10;
      if( year < 57)          /* cycle around Y2K */
         year += 100;
      sat->epoch = atof( line1 + 20) + J1900 + (double)year * 365. +
                           (double)((year - 1) / 4);
      }
   return( rval ? rval : checksum_problem);
}
