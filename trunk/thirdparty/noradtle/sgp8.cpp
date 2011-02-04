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

#define a3cof        (-(xj3)/ck2*(ae*ae*ae))
#define cosi         params[1]
#define cosio2       params[2]
#define ed           params[3]
#define edot         params[4]
#define gamma        params[5]
#define omgdt        params[6]
#define ovgpp        params[7]
#define pp           params[8]
#define qq           params[9]
#define sini         params[10]
#define sinio2       params[11]
#define theta2       params[12]
#define tthmun       params[13]
#define unm5th       params[14]
#define unmth2       params[15]
#define xgdt1        params[16]
#define xhdt1        params[17]
#define xlldot       params[18]
#define xmdt1        params[19]
#define xnd          params[20]
#define xndt         params[21]
#define xnodot       params[22]
#define xnodp        params[23]
#define simple_flag *((int *)( params + 24))

void DLL_FUNC SGP8_init( double *params, const tle_t *tle)
{
   /* Recover original mean motion (xnodp) and semimajor   */
   /* axis (aodp) from input elements. Calculate ballistic */
   /* coefficient (b term) from input b* drag term         */
   const double a1 = pow(xke / tle->xno, two_thirds);
   const double eosq = tle->eo*tle->eo;
   const double betao2 = 1.-eosq;
   const double betao = sqrt(betao2);
   const double b = tle->bstar*2./rho;
   const double sing = sin(tle->omegao);
   const double cosg = cos(tle->omegao);
   const double cos2g = cosg * cosg * 2. - 1.;
   const double half_inclination = tle->xincl*.5;
   double     alpha2, ao, aodp, b1, b2, b3,
              c0, c1, c4, c5,
              d1, d2, d3, d4, d5, del1, delo,
              eddot, eeta, eta, eta2, etdt,
              pardt1, pardt2, pardt4, po, pom2, psim2,
              r1, theta4, tsi, xndtn;

   cosi = cos(tle->xincl);
   theta2 = cosi * cosi;
   tthmun = theta2 * 3. - 1.;
   del1 = ck2*1.5*tthmun/(a1*a1*betao*betao2);
   ao = a1*(1.-del1*(two_thirds*.5 +
                    del1*(del1*1.654320987654321+1.)));
   delo = ck2*1.5*tthmun/(ao*ao*betao*betao2);
   aodp = ao/(1.-delo);
   xnodp = tle->xno/(delo+1.);

   /* Initialization */
   po = aodp*betao2;
   pom2 = 1./(po*po);
   sini = sin(tle->xincl);
   sinio2 = sin( half_inclination);
   cosio2 = cos( half_inclination);
   r1 = theta2;
   theta4 = r1*r1;
   unm5th = 1.-theta2*5.;
   unmth2 = 1.-theta2;
   r1 = ae;
   pardt1 = ck2*3.*pom2*xnodp;
   pardt2 = pardt1*ck2*pom2;
   pardt4 = ck4*1.25*pom2*pom2*xnodp;
   xmdt1 = pardt1*.5*betao*tthmun;
   xgdt1 = pardt1*-.5*unm5th;
   xhdt1 = -pardt1*cosi;
   xlldot = xnodp + xmdt1 + pardt2 * .0625 * betao *
                 (13. - theta2 * 78. + theta4 * 137.);
   omgdt = xgdt1 + pardt2 * .0625 * (7. - theta2 * 114. + theta4*
           395.) + pardt4 * (3. - theta2 * 36. + theta4 * 49.);
   xnodot = xhdt1+(pardt2*.5*(4.-theta2*19.)+
       pardt4 *2.*(3.-theta2*7.))*cosi;
   tsi = 1./(po-s);
   eta = tle->eo*s*tsi;
   r1 = eta;
   eta2 = r1*r1;
   psim2 = (r1 = 1./(1.-eta2), fabs(r1));
   alpha2 = eosq+1.;
   eeta = tle->eo*eta;
   r1 = cosg;
   d5 = tsi*psim2;
   d1 = d5/po;
   d2 = eta2*(eta2*4.5+36.)+12.;
   d3 = eta2*(eta2*2.5+15.);
   d4 = eta*(eta2*3.75+5.);
   b1 = ck2*tthmun;
   b2 = -ck2*unmth2;
   b3 = a3cof*sini;
   r1 = tsi, r1 *= r1;
   c0 = b*.5*rho*qoms2t*xnodp*aodp*(r1*r1)*
                pow(psim2, 3.5)/sqrt(alpha2);
   r1 = alpha2;
   c1 = xnodp*1.5*(r1*r1)*c0;
   c4 = d1*d3*b2;
   c5 = d5*d4*b3;
   xndt = c1*(eta2*(eosq*34.+3.)+2.+eeta*5.*(eta2+4.)
     +eosq*8.5+d1*d2*b1+c4*cos2g+c5*sing);
   xndtn = xndt/xnodp;

   /* If drag is very small, the isimp flag is set and the */
   /* equations are truncated to linear variation in mean  */
   /* motion and quadratic variation in mean anomaly       */
   r1 = xndtn*xmnpda;
   if( fabs(r1) > .00216)
      {
      const double d6 = eta*(eta2*22.5+30.);
      const double d7 = eta*(eta2*12.5+5.);
      const double d8 = eta2*(eta2+6.75)+1.;
      const double d9 = eta*(eosq*68.+6.)+tle->eo*(eta2*15.+20.);
      const double d10 = eta*5.*(eta2+4.)+tle->eo*(eta2*68.+17.);
      const double d11 = eta*(eta2*18.+72.);
      const double d12 = eta*(eta2*10.+30.);
      const double d13 = eta2*11.25+5.;
      const double d20 = two_thirds*.5*xndtn;
      const double c8 = d1*d7*b2;
      const double c9 = d5*d8*b3;
      const double tsdtts = aodp*2.*tsi*(d20*betao2+tle->eo*edot);
      const double sin2g = sing*2.*cosg;
      double d1dt, d2dt, d3dt, d4dt, d5dt, temp;
      double d14, d15, d16, d17, d18, d19, d23, d25, aldtal, psdtps;
      double c4dt, c5dt, c0dtc0, c1dtc1, rr2;
      double d1ddt, etddt, tmnddt, tsddts, xnddt, xntrdt;

      simple_flag = 0;
      edot = -c0*(eta*(eta2+4.+eosq*(eta2*7.+15.5))+tle->eo*
                 (eta2*15.+5.)+d1*d6*b1+c8*cos2g+c9*sing);
      aldtal = tle->eo*edot/alpha2;
      etdt = (edot+tle->eo*tsdtts)*tsi*s;
      psdtps = -eta*etdt*psim2;
      c0dtc0 = d20+tsdtts*4.-aldtal-psdtps*7.;
      c1dtc1 = xndtn+aldtal*4.+c0dtc0;
      d14 = tsdtts-psdtps*2.;
      d15 = (d20+tle->eo*edot/betao2)*2.;
      d1dt = d1*(d14+d15);
      d2dt = etdt*d11;
      d3dt = etdt*d12;
      d4dt = etdt*d13;
      d5dt = d5*d14;
      c4dt = b2*(d1dt*d3+d1*d3dt);
      c5dt = b3*(d5dt*d4+d5*d4dt);
      d16 = d9*etdt+d10*edot+b1*(d1dt*d2+d1*d2dt)+c4dt*
            cos2g+c5dt*sing+xgdt1*(c5*cosg-c4*2.*sin2g);
      xnddt = c1dtc1*xndt+c1*d16;
      eddot = c0dtc0*edot-c0*((eta2*3.+4.+eeta*30.+eosq*
        (eta2*21.+15.5))*etdt+(eta2*15.+5.+eeta*
         (eta2*14.+31.))*edot+b1*(d1dt*d6+d1*etdt*
             (eta2*67.5+30.))+b2*(d1dt*d7+d1*etdt*
        (eta2*37.5+5.))*cos2g+b3*(d5dt*d8+d5*etdt*eta*
        (eta2*4.+13.5))*sing+xgdt1*(c9*cosg-c8*2.*sin2g));
      r1 = edot;
      d25 = r1*r1;
      r1 = xndtn;
      d17 = xnddt/xnodp-r1*r1;
      tsddts = tsdtts*2.*(tsdtts-d20)+aodp*tsi*
               (two_thirds*betao2*d17-d20*4.*tle->eo*edot+
               (d25+tle->eo*eddot)*2.);
      etddt = (eddot+edot*2.*tsdtts)*tsi*s+tsddts*eta;
      r1 = tsdtts;
      d18 = tsddts-r1*r1;
      r1 = psdtps;
      rr2 = psdtps;
      d19 = -(r1*r1)/eta2-eta*etddt*psim2-rr2*rr2;
      d23 = etdt*etdt;
      d1ddt = d1dt*(d14+d15)+d1*(d18-d19*2.+two_thirds*d17+
         (alpha2*d25/betao2+tle->eo*eddot)*2./betao2);
      r1  = aldtal;
      xntrdt = xndt*(two_thirds*2.*d17+(d25+tle->eo*eddot)*3./
               alpha2-r1*r1*6.+d18*4.-d19*7.)+
                    c1dtc1*xnddt+c1*(c1dtc1*d16+d9*etddt+d10*
          eddot+d23*(eeta*30.+6.+eosq*68.)+etdt*edot*
          (eta2*30.+40.+eeta*272.)+d25*(eta2*68.+17.)+
          b1*(d1ddt*d2+d1dt*2.*d2dt+d1*(etddt*d11+d23*
          (eta2*54.+72.)))+b2*(d1ddt*d3+d1dt*2.*d3dt+d1
          *(etddt*d12+d23*(eta2*30.+30.)))*cos2g+b3*
          ((d5dt*d14+d5*(d18-d19*2.))*d4+d4dt*2.*d5dt+
          d5*(etddt*d13+eta*22.5*d23))*sing+xgdt1*((d20*
          7.+tle->eo*4.*edot/betao2)*(c5*cosg-c4*2.*
          sin2g)+(c5dt*2.*cosg-c4dt*4.*sin2g-xgdt1*
          (c5*sing+c4*4.*cos2g))));
      tmnddt = xnddt*1e9;
      r1 = tmnddt;
      temp = r1*r1-xndt*1e18*xntrdt;
      r1 = tmnddt;
      pp = (temp+r1*r1)/temp;
      gamma = -xntrdt/(xnddt*(pp-2.));
      xnd = xndt/(pp*gamma);
      qq = 1.-eddot/(edot*gamma);
      ed = edot/(qq*gamma);
      ovgpp = 1./(gamma*(pp+1.));
      }
   else
      {
      simple_flag = 1;
      edot = -two_thirds*xndtn*(1.-tle->eo);
      }  /* End of if (fabs(r1) > .00216) */
} /* End of SGP8() initialization */

void DLL_FUNC SGP8( const double tsince, const tle_t *tle, const double *params,
                                           double *pos, double *vel)
{
   int i;
   double
        am, aovr, axnm, aynm, beta, beta2m,
        cose, cosos, cs2f2g, csf, csfg,
        cslamb, di, diwc, dr, ecosf, em, fm,
        g1, g10, g13, g14, g2, g3, g4, g5,
        omgasm, pm, r1, rdot, rm, rr, rvdot,
        sine, sinos, sn2f2g, snf, snfg,
        sni2du, snlamb, temp, ux, uy,
        uz, vx, vy, vz, xlamb, xmam, xn,
        xnodes, y4, y5, z1, z7, zc2, zc5;

  /* Update for secular gravity and atmospheric drag */
  r1 = tle->xmo+xlldot*tsince;
  xmam = FMod2p(r1);
  omgasm = tle->omegao+omgdt*tsince;
  xnodes = tle->xnodeo+xnodot*tsince;
  if( !simple_flag)
    {
      double temp1;

      temp = 1.-gamma*tsince;
      temp1 = pow(temp, pp);
      xn = xnodp+xnd*(1.-temp1);
      em = tle->eo+ed*(1.-pow(temp, qq));
      z1 = xnd*(tsince+ovgpp*(temp*temp1-1.));
    }
  else
    {
      xn = xnodp+xndt*tsince;
      em = tle->eo+edot*tsince;
      z1 = xndt*.5*tsince*tsince;
    }  /* if(isFlagClear(SIMPLE_FLAG)) */

  z7 = two_thirds*3.5*z1/xnodp;
  r1 = xmam+z1+z7*xmdt1;
  xmam = FMod2p(r1);
  omgasm += z7*xgdt1;
  xnodes += z7*xhdt1;

  /* Solve Kepler's equation */
  zc2 = xmam+em*sin(xmam)*(em*cos(xmam)+1.);

  i = 0;
  do
    {
      double cape;

      sine = sin(zc2);
      cose = cos(zc2);
      zc5 = 1./(1.-em*cose);
      cape = (xmam+em*sine-zc2)*zc5+zc2;
      r1 = cape-zc2;
      if(fabs(r1) <= e6a) break;
      zc2 = cape;
    }
  while(i++ < 10 );

  /* Short period preliminary quantities */
  am = pow( xke / xn, two_thirds);
  beta2m = 1.-em*em;
  sinos = sin(omgasm);
  cosos = cos(omgasm);
  axnm = em*cosos;
  aynm = em*sinos;
  pm = am*beta2m;
  g1 = 1./pm;
  g2 = ck2*.5*g1;
  g3 = g2*g1;
  beta = sqrt(beta2m);
  g4 = a3cof * .25 * sini;
  g5 = a3cof * .25 * g1;
  snf = beta*sine*zc5;
  csf = (cose-em)*zc5;
  fm = atan2(snf, csf);
  if( fm < 0.)
     fm += pi + pi;
  snfg = snf*cosos+csf*sinos;
  csfg = csf*cosos-snf*sinos;
  sn2f2g = snfg*2.*csfg;
  r1 = csfg;
  cs2f2g = r1*r1*2.-1.;
  ecosf = em*csf;
  g10 = fm-xmam+em*snf;
  rm = pm/(ecosf+1.);
  aovr = am/rm;
  g13 = xn*aovr;
  g14 = -g13*aovr;
  dr = g2*(unmth2*cs2f2g-tthmun*3.)-g4*snfg;
  diwc = g3*3.*sini*cs2f2g-g5*aynm;
  di = diwc*cosi;

  /* Update for short period periodics */
  sni2du = sinio2*(g3*((1.-theta2*7.)*.5*sn2f2g-unm5th*3.*g10)-
      g5*sini*csfg*(ecosf+2.))-g5*.5f*theta2*axnm/cosio2;
  xlamb = fm+omgasm+xnodes+g3*((cosi*6.+1.-theta2*7.)*
     .5*sn2f2g-(unm5th+cosi*2.)*3.*g10)+g5*sini*
          (cosi*axnm/(cosi+1.)-(ecosf+2.)*csfg);
  y4 = sinio2*snfg+csfg*sni2du+snfg*.5*cosio2*di;
  y5 = sinio2*csfg-snfg*sni2du+csfg*.5*cosio2*di;
  rr = rm+dr;
  rdot = xn*am*em*snf/beta+g14*(g2*2.*unmth2*sn2f2g+g4*csfg);
  r1 = am;
  rvdot = xn*(r1*r1)*beta/rm+g14 *
          dr+am*g13*sini*diwc;

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

} /* SGP8 */
