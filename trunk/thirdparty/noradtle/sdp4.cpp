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

#define x3thm1       params[0]
#define x1mth2       params[1]
#define c1           params[2]
#define c4           params[3]
#define xnodcf       params[4]
#define t2cof        params[5]
#define xlcof        params[6]
#define aycof        params[7]
#define x7thm1       params[8]
#define deep_arg     ((deep_arg_t *)( params + 9))

void DLL_FUNC SDP4_init( double *params, const tle_t *tle)
{
   init_t init;

   sxpx_common_init( params, tle, &init, deep_arg);
   deep_arg->sing = sin(tle->omegao);
   deep_arg->cosg = cos(tle->omegao);

   /* initialize Deep() */
   Deep_dpinit( tle, deep_arg);
#ifdef RETAIN_PERTURBATION_VALUES_AT_EPOCH
   /* initialize lunisolar perturbations: */
   deep_arg->t = 0.;                            /* added 30 Dec 2003 */
   deep_arg->solar_lunar_init_flag = 1;
   Deep_dpper( deep_arg);
   deep_arg->solar_lunar_init_flag = 0;
#endif
} /*End of SDP4() initialization */

void DLL_FUNC SDP4( const double tsince, const tle_t *tle, const double *params,
                                         double *pos, double *vel)
{
  double
      a, tempa, tempe, templ, tsq,
      xl, xmam, xmdf, xnoddf;

  /* Update for secular gravity and atmospheric drag */
  xmdf = tle->xmo+deep_arg->xmdot*tsince;
  deep_arg->omgadf = tle->omegao+deep_arg->omgdot*tsince;
  xnoddf = tle->xnodeo+deep_arg->xnodot*tsince;
  tsq = tsince*tsince;
  deep_arg->xnode = xnoddf+xnodcf*tsq;
  tempa = 1-c1*tsince;
  tempe = tle->bstar*c4*tsince;
  templ = t2cof*tsq;
  deep_arg->xn = deep_arg->xnodp;

  /* Update for deep-space secular effects */
  deep_arg->xll = xmdf;
  deep_arg->t = tsince;

  Deep_dpsec( tle, deep_arg);

  xmdf = deep_arg->xll;
  a = pow(xke/deep_arg->xn,two_thirds)*tempa*tempa;
  deep_arg->em = deep_arg->em-tempe;
  xmam = xmdf+deep_arg->xnodp*templ;

  /* Update for deep-space periodic effects */
  deep_arg->xll = xmam;

  Deep_dpper( deep_arg);

  xmam = deep_arg->xll;
  xl = xmam + deep_arg->omgadf + deep_arg->xnode;
  sxpx_posn_vel( deep_arg->xnode, a, deep_arg->em, params, deep_arg->cosio,
                deep_arg->sinio, deep_arg->xinc, deep_arg->omgadf,
                xl, pos, vel);
} /* SDP4 */
