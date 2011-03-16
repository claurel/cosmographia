// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

#include "AstorbLoader.h"
#include "../astro/Constants.h"
#include <vesta/Units.h>
#include <vesta/GregorianDate.h>
#include <QFile>
#include <QDebug>
#include <QRegExp>

using namespace vesta;


/** Load a text file containing minor planet data in the ASTORB format used in
  * the catalog maintained by Ted Bowell. Information about the format and a link
  * to the must current data is here:
  *
  *   ftp://ftp.lowell.edu/pub/elgb/astorb.html
  */
KeplerianSwarm*
LoadAstorbFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Unable to open astorb data file " << fileName;
        return NULL;
    }

    KeplerianSwarm* swarm = new KeplerianSwarm();

    unsigned int objectCount = 0;

    QRegExp provisionalDesignation("\\d\\d\\d\\d [A-Z][A-Z]\\d*");

    QTextStream in(&file);
    while (!in.atEnd() && in.status() == QTextStream::Ok)
    {
        QString record = in.readLine();
        if (in.status() == QTextStream::Ok)
        {
            QString epochYear = record.mid(106, 4);
            QString epochMonth = record.mid(110, 2);
            QString epochDay = record.mid(112, 2);
            QString meanAnomaly = record.mid(115, 10);
            QString argOfPeri = record.mid(126, 10);
            QString ascendingNode = record.mid(137, 10);
            QString inclination = record.mid(148, 9);
            QString eccentricity = record.mid(158, 10);
            QString sma = record.mid(169, 12);

            QString name = record.mid(7, 19).trimmed();

            //bool isNEO = el.periapsisDistance / AU < 1.3;
            //bool isHilda = el.inclination < toRadians(20.0) && el.eccentricity < 0.3 && smaAU > 3.7 && smaAU < 4.1;
            //bool isJupiterTrojan = smaAU > 5.1 && smaAU < 5.35 && el.eccentricity < 0.25;
            //bool isKBO = smaAU >= 30.0;


            double discoveryTime = -daysToSeconds(365.25 * 100);
            if (provisionalDesignation.indexIn(name) == 0)
            {
                double year = name.mid(0, 4).toDouble();
                double halfMonth = name.at(5).toAscii() - 'A';
                discoveryTime = (year - 2000.0) * 365.25 + halfMonth * (365.25 / 24.0);
                discoveryTime *= 86400.0;
            }

            // float absMag = record.mid(43, 5).toFloat();

            // Epoch is Terrestrial Time
            GregorianDate epoch(epochYear.toInt(), epochMonth.toInt(), epochDay.toInt(), 12, 0, 0);
            epoch.setTimeScale(TimeScale_TT);
            double smaAU = sma.toDouble();
            double periodYears = pow(smaAU, 1.5);

            OrbitalElements el;
            el.eccentricity = eccentricity.toDouble();
            el.periapsisDistance = (1.0 - el.eccentricity) * smaAU * astro::AU;
            el.inclination = toRadians(inclination.toDouble());
            el.longitudeOfAscendingNode = toRadians(ascendingNode.toDouble());
            el.argumentOfPeriapsis = toRadians(argOfPeri.toDouble());
            el.meanAnomalyAtEpoch = toRadians(meanAnomaly.toDouble());
            el.meanMotion = 2.0 * PI / daysToSeconds(365.25 * periodYears);
            el.epoch = epoch.toTDBSec();

            // Automatically set epoch for the swarm geometry to that of the first record in the file
            if (objectCount == 0)
            {
                swarm->setEpoch(el.epoch);
            }

            swarm->addObject(el, discoveryTime);
            objectCount++;
        }
    }

    if (objectCount == 0)
    {
        qDebug() << "astorb file " << fileName << " contains no records";
        delete swarm;
        swarm = NULL;
    }

    return swarm;
}
