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

#define tthmun    params[0]
#define sinio2    params[1]
#define cosio2    params[2]
#define unm5th    params[3]
#define unmth2    params[4]
#define a3cof     params[5]
#define xmdt1     params[6]
#define xgdt1     params[7]
#define xhdt1     params[8]
#define xndt      params[9]
#define edot      params[10]
#define deep_arg  ((deep_arg_t *)( params + 11))

void DLL_FUNC SDP8_init( double *params, const tle_t *tle)
{
   double
         a1, alpha2, ao, b, b1, b2, b3, c0,
         c1, c4, c5, cos2g, d1, d2, d3, d4,
         d5, del1, delo, eeta, eta, eta2,
         pardt1, pardt2, pardt4, po, pom2,
         psim2, r1, temp, theta4, tsi, xndtn;

   /* Recover original mean motion (xnodp) and semimajor axis (aodp) */
   /* from input elements. Calculate ballistic coefficient */
   /* (b term) from input b* drag term */
   a1 = pow( xke / tle->xno, two_thirds);
   deep_arg->cosio = cos(tle->xincl);
   deep_arg->theta2 = deep_arg->cosio*deep_arg->cosio;
   tthmun = deep_arg->theta2*3.-1.;
   deep_arg->eosq = tle->eo*tle->eo;
   deep_arg->betao2 = 1.-deep_arg->eosq;
   deep_arg->betao = sqrt(deep_arg->betao2);
   del1 = ck2*1.5*tthmun/(a1*a1*deep_arg->betao*deep_arg->betao2);
   ao = a1*(1.-del1*(two_thirds*.5+del1*(del1*1.654320987654321+1.)));
   delo = ck2*1.5*tthmun/(ao*ao*deep_arg->betao*deep_arg->betao2);
   deep_arg->aodp = ao/(1.-delo);
   deep_arg->xnodp = tle->xno/(delo+1.);
   b = tle->bstar*2./rho;

   /* Initialization */
   po = deep_arg->aodp*deep_arg->betao2;
   pom2 = 1./(po*po);
   deep_arg->sinio = sin(tle->xincl);
   deep_arg->sing = sin(tle->omegao);
   deep_arg->cosg = cos(tle->omegao);
   temp = tle->xincl*.5;
   sinio2 = sin(temp);
   cosio2 = cos(temp);
   r1 = deep_arg->theta2;
   theta4 = r1*r1;
   unm5th = 1.-deep_arg->theta2*5.;
   unmth2 = 1.-deep_arg->theta2;
   r1 = ae;
   a3cof = -xj3/ck2*(r1*(r1*r1));
   pardt1 = ck2*3.*pom2*deep_arg->xnodp;
   pardt2 = pardt1*ck2*pom2;
   pardt4 = ck4*1.25*pom2*pom2*deep_arg->xnodp;
   xmdt1 = pardt1*.5*deep_arg->betao*tthmun;
   xgdt1 = pardt1*-.5*unm5th;
   xhdt1 = -pardt1*deep_arg->cosio;
   deep_arg->xmdot = deep_arg->xnodp+xmdt1+pardt2*.0625*deep_arg->betao*
               (13.-deep_arg->theta2*78.+theta4*137.);
   deep_arg->omgdot = xgdt1+pardt2*.0625*(7.-deep_arg->theta2*
                     114.+theta4*395.)+pardt4*(3.-deep_arg->theta2*
                     36.+theta4*49.);
   deep_arg->xnodot = xhdt1+(pardt2*.5*(4.-deep_arg->theta2*19.)+pardt4*
      2.*(3.-deep_arg->theta2*7.))*deep_arg->cosio;
   tsi = 1./(po-s);
   eta = tle->eo*s*tsi;
   r1 = eta;
   eta2 = r1*r1;
   psim2 = (r1 = 1./(1.-eta2), fabs(r1));
   alpha2 = deep_arg->eosq+1.;
   eeta = tle->eo*eta;
   r1 = deep_arg->cosg;
   cos2g = r1*r1*2.-1.;
   d5 = tsi*psim2;
   d1 = d5/po;
   d2 = eta2*(eta2*4.5+36.)+12.;
   d3 = eta2*(eta2*2.5+15.);
   d4 = eta*(eta2*3.75+5.);
   b1 = ck2*tthmun;
   b2 = -ck2*unmth2;
   b3 = a3cof*deep_arg->sinio;
   r1 = tsi, r1 *= r1;
   c0 = b*.5*rho*qoms2t*deep_arg->xnodp*deep_arg->aodp*
            (r1*r1)*pow( psim2, 3.5)/sqrt(alpha2);
   r1 = alpha2;
   c1 = deep_arg->xnodp*1.5*(r1*r1)*c0;
   c4 = d1*d3*b2;
   c5 = d5*d4*b3;
   xndt = c1*(eta2*(deep_arg->eosq*34.+3.)+2.+eeta*5.*(eta2+4.)+
     deep_arg->eosq*8.5+d1*d2*b1+c4*cos2g+c5*deep_arg->sing);
   xndtn = xndt/deep_arg->xnodp;
   edot = -two_thirds*xndtn*(1.-tle->eo);

   /* initialize Deep() */
   Deep_dpinit( tle, deep_arg);
#ifdef RETAIN_PERTURBATION_VALUES_AT_EPOCH
   /* initialize lunisolar perturbations: */
   deep_arg->t = 0.;                            /* added 30 Dec 2003 */
   deep_arg->solar_lunar_init_flag = 1;
   Deep_dpper( deep_arg);
   deep_arg->solar_lunar_init_flag = 0;
#endif
} /* End of SDP8() initialization */

void DLL_FUNC SDP8( const double tsince, const tle_t *tle, const double *params,
                                double *pos, double *vel)
{
   double
        am, aovr, axnm, aynm, beta, beta2m, cape,
        cose, cosos, cs2f2g, csf, csfg, cslamb, di,
        diwc, dr, ecosf, fm, g1, g10, g13, g14, g2,
        g3, g4, g5, pm, r1, rdot, rm, rr, rvdot, sine,
        sini2, sinos, sn2f2g, snf, snfg, sni2du,
        snlamb, temp, ux, uy, uz, vx, vy, vz, xlamb,
        xmam, xmamdf, y4, y5, z1, z7, zc2, zc5;
  int i;

  /* Update for secular gravity and atmospheric drag */
  z1 = xndt*.5*tsince*tsince;
  z7 = two_thirds*3.5*z1/deep_arg->xnodp;
  xmamdf = tle->xmo+deep_arg->xmdot*tsince;
  deep_arg->omgadf = tle->omegao+deep_arg->omgdot*tsince+z7*xgdt1;
  deep_arg->xnode = tle->xnodeo+deep_arg->xnodot*tsince+z7*xhdt1;
  deep_arg->xn = deep_arg->xnodp;

  /* Update for deep-space secular effects */
  deep_arg->xll = xmamdf;
  deep_arg->t = tsince;
  Deep_dpsec( tle, deep_arg);
  xmamdf = deep_arg->xll;
  deep_arg->xn += xndt*tsince;
  deep_arg->em += edot*tsince;
  xmam = xmamdf+z1+z7*xmdt1;

  /* Update for deep-space periodic effects */
  deep_arg->xll = xmam;
  Deep_dpper( deep_arg);
  xmam = deep_arg->xll;
  xmam = FMod2p(xmam);

  /* Solve Kepler's equation */
  zc2 = xmam+deep_arg->em*sin(xmam)*(deep_arg->em*cos(xmam)+1.);

  i = 0;
  do
    {
      sine = sin(zc2);
      cose = cos(zc2);
      zc5 = 1./(1.-deep_arg->em*cose);
      cape = (xmam+deep_arg->em*sine-zc2)*zc5+zc2;
      r1 = cape-zc2;
      if (fabs(r1) <= e6a) break;
      zc2 = cape;
    }
  while(i++ < 10);

  /* Short period preliminary quantities */
  am = pow( xke / deep_arg->xn, two_thirds);
  beta2m = 1.f-deep_arg->em*deep_arg->em;
  sinos = sin(deep_arg->omgadf);
  cosos = cos(deep_arg->omgadf);
  axnm = deep_arg->em*cosos;
  aynm = deep_arg->em*sinos;
  pm = am*beta2m;
  g1 = 1./pm;
  g2 = ck2*.5*g1;
  g3 = g2*g1;
  beta = sqrt(beta2m);
  g4 = a3cof*.25*deep_arg->sinio;
  g5 = a3cof*.25*g1;
  snf = beta*sine*zc5;
  csf = (cose-deep_arg->em)*zc5;
  fm = atan2(snf, csf);
  if( fm < 0.)
     fm += pi + pi;
  snfg = snf*cosos+csf*sinos;
  csfg = csf*cosos-snf*sinos;
  sn2f2g = snfg*2.*csfg;
  r1 = csfg;
  cs2f2g = r1*r1*2.-1.;
  ecosf = deep_arg->em*csf;
  g10 = fm-xmam+deep_arg->em*snf;
  rm = pm/(ecosf+1.);
  aovr = am/rm;
  g13 = deep_arg->xn*aovr;
  g14 = -g13*aovr;
  dr = g2*(unmth2*cs2f2g-tthmun*3.)-g4*snfg;
  diwc = g3*3.*deep_arg->sinio*cs2f2g-g5*aynm;
  di = diwc*deep_arg->cosio;
  sini2 = sin(deep_arg->xinc*.5);

  /* Update for short period periodics */
  sni2du = sinio2*(g3*((1.-deep_arg->theta2*7.)*.5*sn2f2g-unm5th*
      3.*g10)-g5*deep_arg->sinio*csfg*(ecosf+2.))-g5*.5*
           deep_arg->theta2*axnm/cosio2;
  xlamb = fm+deep_arg->omgadf+deep_arg->xnode+g3*((deep_arg->cosio*6.+
     1.-deep_arg->theta2*7.)*.5*sn2f2g-(unm5th+deep_arg->cosio*2.)*
     3.*g10)+g5*deep_arg->sinio*(deep_arg->cosio*axnm/
     (deep_arg->cosio+1.)-(ecosf+2.)*csfg);
  y4 = sini2*snfg+csfg*sni2du+snfg*.5*cosio2*di;
  y5 = sini2*csfg-snfg*sni2du+csfg*.5*cosio2*di;
  rr = rm+dr;
  rdot = deep_arg->xn*am*deep_arg->em*snf/beta+g14*(g2*2.*unmth2*sn2f2g+g4*csfg);
  r1 = am;
  rvdot = deep_arg->xn*(r1*r1)*beta/rm+g14*dr+am*g13*deep_arg->sinio*diwc;

  /* Orientation vectors */
  snlamb = sin(xlamb);
  cslamb = cos(xlamb);
  temp = (y5*snlamb-y4*cslamb)*2.;
  ux = y4*temp+cslamb;
  vx = y5*temp-snlamb;
  temp = (y5*cslamb+y4*snlamb)*2.;
  uy = -y4*temp+snlamb;
  vy = -y5*temp+cslamb;
  temp = sqrt(1.-y4*y4-y5*y5)*2.;
  uz = y4*temp;
  vz = y5*temp;

  /* Position and velocity */
  pos[0] = rr*ux*xkmper;
  pos[1] = rr*uy*xkmper;
  pos[2] = rr*uz*xkmper;
  if( vel)
     {
     vel[0] = (rdot*ux+rvdot*vx)*xkmper;
     vel[1] = (rdot*uy+rvdot*vy)*xkmper;
     vel[2] = (rdot*uz+rvdot*vz)*xkmper;
     }

} /* SDP8 */
