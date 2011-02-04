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



#include <math.h>
#include "norad.h"
#include "norad_in.h"

/*------------------------------------------------------------------*/

/* FMOD2P */
double FMod2p( const double x)
{
   double rval = fmod( x, twopi);

   if( rval < 0.)
      rval += twopi;
   return( rval);
} /* fmod2p */

/*------------------------------------------------------------------*/

/* Selects the type of ephemeris to be used (SGP*-SDP*) */
int DLL_FUNC select_ephemeris( const tle_t *tle)
{
   double ao, xnodp, delo, a1, del1, r1, temp;
   int rval;

   /* Period > 225 minutes is deep space */
   a1 = pow( xke / tle->xno, two_thirds);
   r1 = cos(tle->xincl);
   temp = ck2 * 1.5 * (r1*r1*3.0-1.0) * pow( 1.0-tle->eo*tle->eo, -1.5);
   del1 = temp/(a1*a1);
   ao = a1 * (1.0 - del1 * (1./3. + del1 * (del1 * 1.654320987654321+1.0)));
   delo = temp/(ao*ao);
   xnodp = tle->xno / (delo + 1.0);

   /* Select a deep-space/near-earth ephemeris */
   /* If the object makes less than 6.4 revolutions around the earth... */
   if (twopi / (xnodp * xmnpda) >= (1. / 6.4))
      rval = 1;      /* yes,  it should be a deep-space (SDPx) ephemeris */
   else
      rval = 0;      /* no,  you can go with an SGPx ephemeris */
   return( rval);
} /* End of select_ephemeris() */

/*------------------------------------------------------------------*/
