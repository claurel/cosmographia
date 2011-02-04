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
#include <stdio.h>
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

void sxpx_common_init( double *params, const tle_t *tle,
                                  init_t *init, deep_arg_t *deep_arg)
{
   double
         a1, ao, c2, del1,
         delo, eeta, etasq, perige, pinvsq,
         psisq, qoms24, temp1, temp2, temp3,
         theta4, tsi_squared, x1m5th, xhdot1;

   /* Recover original mean motion (xnodp) and   */
   /* semimajor axis (aodp) from input elements. */
   a1 = pow(xke/tle->xno,two_thirds);
   deep_arg->cosio = cos(tle->xincl);
   deep_arg->theta2 = deep_arg->cosio*deep_arg->cosio;
   x3thm1 = 3*deep_arg->theta2-1;
   deep_arg->eosq = tle->eo*tle->eo;
   deep_arg->betao2 = 1-deep_arg->eosq;
   deep_arg->betao = sqrt(deep_arg->betao2);
   del1 = 1.5*ck2*x3thm1/(a1*a1*deep_arg->betao*deep_arg->betao2);
   ao = a1*(1-del1*(0.5*two_thirds+del1*(1.+134./81.*del1)));
   delo = 1.5*ck2*x3thm1/(ao*ao*deep_arg->betao*deep_arg->betao2);
   deep_arg->xnodp = tle->xno/(1+delo);
   deep_arg->aodp = ao/(1-delo);

   /* For perigee below 156 km, the values */
   /* of s and qoms2t are altered.         */
   init->s4 = s;
   qoms24 = qoms2t;
   perige = (deep_arg->aodp*(1-tle->eo)-ae)*xkmper;
   if(perige < 156)
      {
      double temp_val, temp_val_squared;

      if(perige <= 98)
         init->s4 = 20;
      else
         init->s4 = perige-78;
      temp_val = (120. - init->s4) * ae / xkmper;
      temp_val_squared = temp_val * temp_val;
      qoms24 = temp_val_squared * temp_val_squared;
      init->s4 = init->s4/xkmper+ae;
      }  /* End of if(perige <= 156) */

   pinvsq = 1/(deep_arg->aodp*deep_arg->aodp*
            deep_arg->betao2*deep_arg->betao2);
   init->tsi = 1/(deep_arg->aodp-init->s4);
   init->eta = deep_arg->aodp*tle->eo*init->tsi;
   etasq = init->eta*init->eta;
   eeta = tle->eo*init->eta;
   psisq = fabs(1-etasq);
   tsi_squared = init->tsi * init->tsi;
   init->coef = qoms24 * tsi_squared * tsi_squared;
   init->coef1 = init->coef / pow(psisq,3.5);
   c2 = init->coef1 * deep_arg->xnodp * (deep_arg->aodp*(1+1.5*etasq+eeta*
   (4+etasq))+0.75*ck2*init->tsi/psisq*x3thm1*(8+3*etasq*(8+etasq)));
   c1 = tle->bstar*c2;
   deep_arg->sinio = sin(tle->xincl);
   init->a3ovk2 = -xj3/ck2*ae*ae*ae;
   x1mth2 = 1-deep_arg->theta2;
   c4 = 2*deep_arg->xnodp*init->coef1*deep_arg->aodp*deep_arg->betao2*
        (init->eta*(2+0.5*etasq)+tle->eo*(0.5+2*etasq)-2*ck2*init->tsi/
        (deep_arg->aodp*psisq)*(-3*x3thm1*(1-2*eeta+etasq*
        (1.5-0.5*eeta))+0.75*x1mth2*(2*etasq-eeta*(1+etasq))*
        cos(2*tle->omegao)));
   theta4 = deep_arg->theta2*deep_arg->theta2;
   temp1 = 3*ck2*pinvsq*deep_arg->xnodp;
   temp2 = temp1*ck2*pinvsq;
   temp3 = 1.25*ck4*pinvsq*pinvsq*deep_arg->xnodp;
   deep_arg->xmdot = deep_arg->xnodp+0.5*temp1*deep_arg->betao*
               x3thm1+0.0625*temp2*deep_arg->betao*
                    (13-78*deep_arg->theta2+137*theta4);
   x1m5th = 1-5*deep_arg->theta2;
   deep_arg->omgdot = -0.5*temp1*x1m5th+0.0625*temp2*
                     (7-114*deep_arg->theta2+395*theta4)+
                temp3*(3-36*deep_arg->theta2+49*theta4);
   xhdot1 = -temp1*deep_arg->cosio;
   deep_arg->xnodot = xhdot1+(0.5*temp2*(4-19*deep_arg->theta2)+
           2*temp3*(3-7*deep_arg->theta2))*deep_arg->cosio;
   xnodcf = 3.5*deep_arg->betao2*xhdot1*c1;
   t2cof = 1.5*c1;
   xlcof = 0.125*init->a3ovk2*deep_arg->sinio*(3+5*deep_arg->cosio)/
           (1+deep_arg->cosio);
   aycof = 0.25*init->a3ovk2*deep_arg->sinio;
   x7thm1 = 7*deep_arg->theta2-1;
}

void sxpx_posn_vel( const double xnode, const double a, const double e,
      const double *params, const double cosio, const double sinio,
      const double xincl, const double omega,
      const double xl, double *pos, double *vel)
{
  /* Long period periodics */
   const double axn = e*cos(omega);
   double temp = 1/(a*(1.-e*e));
   const double xll = temp*xlcof*axn;
   const double aynl = temp*aycof;
   const double xlt = xl+xll;
   const double ayn = e*sin(omega)+aynl;
   const double elsq = axn*axn+ayn*ayn;
   const double capu = FMod2p(xlt-xnode);
   double temp1, temp2, temp3, temp4, temp5, temp6;
   double ecose, esine, pl, r;
   double betal;
   double u, sinu, cosu, sin2u, cos2u;
   double rk, uk, xnodek, xinck;
   double sinuk, cosuk, sinik, cosik, sinnok, cosnok, xmx, xmy;
   double sinepw, cosepw;
   double ux, uy, uz;
   int i;

                /* Added 29 Mar 2003:  extremely decayed satellites can    */
                /* end up "orbiting" within the earth, and then with a < 0 */
                /* or q < 0.  If evaluating the state vector would lead to */
                /* a math error,  we set a zero posn/vel and quit.         */
                /* Revised 16 Oct 2004 to catch a few other "problem cases" */

   if( a <= 0. || a * (1. - e) <= 0. || elsq >= 1.)
      {
      printf( "ERROR: a = %lf; e = %lf\n", a, e);
      for( i = 0; i < 3; i++)
         {
         pos[i] = 0.;
         if( vel)
            vel[i] = 0.;
         }
      return;
      }
  /* Solve Kepler's' Equation */
   i = 0;
   temp2 = capu;
   do
      {
      double epw;

      sinepw = sin(temp2);
      cosepw = cos(temp2);
      temp3 = axn*sinepw;
      temp4 = ayn*cosepw;
      temp5 = axn*cosepw;
      temp6 = ayn*sinepw;
      epw = (capu-temp4+temp3-temp2)/(1-temp5-temp6)+temp2;
      if(fabs(epw-temp2) <= e6a) break;
      temp2 = epw;
      }
   while( i++ < 10 );

  /* Short period preliminary quantities */
   ecose = temp5+temp6;
   esine = temp3-temp4;
   temp = 1-elsq;
   pl = a*temp;
   r = a*(1-ecose);
   temp1 = 1/r;
   temp2 = a*temp1;
   betal = sqrt(temp);
   temp3 = 1/(1+betal);
   cosu = temp2*(cosepw-axn+ayn*esine*temp3);
   sinu = temp2*(sinepw-ayn-axn*esine*temp3);
   u = atan2( sinu, cosu);
   sin2u = 2*sinu*cosu;
   cos2u = 2*cosu*cosu-1;
   temp = 1/pl;
   temp1 = ck2*temp;
   temp2 = temp1*temp;

  /* Update for short periodics */
   rk = r*(1-1.5*temp2*betal*x3thm1)+0.5*temp1*x1mth2*cos2u;
   uk = u-0.25*temp2*x7thm1*sin2u;
   xnodek = xnode+1.5*temp2*cosio*sin2u;
   xinck = xincl+1.5*temp2*cosio*sinio*cos2u;

  /* Orientation vectors */
   sinuk = sin(uk);
   cosuk = cos(uk);
   sinik = sin(xinck);
   cosik = cos(xinck);
   sinnok = sin(xnodek);
   cosnok = cos(xnodek);
   xmx = -sinnok*cosik;
   xmy = cosnok*cosik;
   ux = xmx*sinuk+cosnok*cosuk;
   uy = xmy*sinuk+sinnok*cosuk;
   uz = sinik*sinuk;

  /* Position and velocity */
   pos[0] = rk*ux*xkmper;
   pos[1] = rk*uy*xkmper;
   pos[2] = rk*uz*xkmper;
   if( vel)
      {
      const double rdot = xke*sqrt(a)*esine/r;
      const double rfdot = xke*sqrt(pl)/r;
      const double xn = xke/(a * sqrt(a));
      const double rdotk = rdot-xn*temp1*x1mth2*sin2u;
      const double rfdotk = rfdot+xn*temp1*(x1mth2*cos2u+1.5*x3thm1);
      const double vx = xmx*cosuk-cosnok*sinuk;
      const double vy = xmy*cosuk-sinnok*sinuk;
      const double vz = sinik*cosuk;

      vel[0] = (rdotk*ux+rfdotk*vx)*xkmper;
      vel[1] = (rdotk*uy+rfdotk*vy)*xkmper;
      vel[2] = (rdotk*uz+rfdotk*vz)*xkmper;
      }
} /*SGP4*/
