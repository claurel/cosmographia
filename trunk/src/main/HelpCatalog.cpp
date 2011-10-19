// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
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

#include "HelpCatalog.h"
#include "UnitConversion.h"
#include "catalog/UniverseCatalog.h"
#include <vesta/Geometry.h>
#include <vesta/Arc.h>
#include <vesta/Trajectory.h>
#include <vesta/RotationModel.h>
#include <vesta/Units.h>
#include <QDir>
#include <QDebug>
#include <algorithm>
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;


/** Create a new HelpCatalog. The catalog does not take ownership
  * of the universe catalog pointer.
  */
HelpCatalog::HelpCatalog(UniverseCatalog* universeCatalog, QObject* parent) :
    QObject(parent),
    m_universeCatalog(universeCatalog)
{
}


HelpCatalog::~HelpCatalog()
{
}


/** Load all HTML files in the specified path. Returns the number
  * of files successfully loaded.
  */
int
HelpCatalog::loadHelpFiles(const QString& path)
{
    QDir dir(path);

    QStringList filters;
    filters << "*.html" << "*.htm";

    int filesLoaded = 0;

    QFileInfoList helpFiles = dir.entryInfoList(filters);
    foreach (QFileInfo fileInfo, helpFiles)
    {
        QFile htmlFile(fileInfo.filePath());
        if (htmlFile.open(QIODevice::ReadOnly))
        {
            QString resourceName = fileInfo.baseName();
            QString contents = QString::fromUtf8(htmlFile.readAll());

            QString dataPageLink = QString("<a href=\"help:%1?data\">Data</a><br>").arg(resourceName);
            contents.replace("<!-- DATA -->", dataPageLink);

            m_helpResources[resourceName] = contents;
        }
    }

    return filesLoaded;
}


/** Return the named help resource. Returns an empty string if the
  * resource isn't available.
  */
QString
HelpCatalog::getHelpText(const QString& name) const
{
    if (name.endsWith("?data"))
    {
        return getObjectDataText(name.section('?', 0, 0));
    }

    QString help = m_helpResources.value(name.toLower());
    if (help.isEmpty())
    {
        // No help available; see if the named object has a custom info resource and use
        // that. If not, create a default info page.
        Entity* body = m_universeCatalog->find(name, Qt::CaseInsensitive);
        if (body != NULL)
        {
            BodyInfo* info = m_universeCatalog->findInfo(body);
            if (info)
            {
                if (info->infoSource.startsWith("help:"))
                {
                    help = m_helpResources.value(info->infoSource.mid(5));
                }
                else if (!info->infoSource.isEmpty())
                {
                    QFile infoFile(info->infoSource);
                    if (infoFile.open(QFile::ReadOnly))
                    {
                        help = QString::fromLatin1(infoFile.readAll());
                    }
                }
            }
        }

        // Nothing worked. Create a default help page if an object with
        // the specified name is present in the catalog.
        if (help.isEmpty())
        {
            if (body != NULL)
            {
                QString description = m_universeCatalog->getDescription(body);
                help = QString("<h1>%1</h1>%2").arg(QString::fromUtf8(body->name().c_str()), description);
            }
            else
            {
                help = QString("<h1>%1</h1>No information available").arg(name);
            }
        }
    }

    return help;
}


static
QString formatDuration(double seconds)
{
    double days = secondsToDays(seconds);
    double years = days / 365.25;

    if (days > 100.0)
    {
        return QObject::tr("%1 years").arg(years);
    }
    else if (days > 2.0)
    {
        return QObject::tr("%1 days").arg(days);
    }
    else
    {
        int hours = int(seconds / 3600.0);
        double minutes = (seconds - hours * 3600.0) / 60.0;
        double seconds = (minutes - int(minutes)) * 60.0;

        if (hours >= 1)
        {
            return QObject::tr("%1h %2m %3s").arg(hours).arg(int(minutes)).arg(int(seconds));
        }
        else
        {
            return QObject::tr("%1m %2s").arg(int(minutes)).arg(int(seconds));
        }
    }
}


static
double roundToSigDigits(double value, int significantDigits)
{
    if (value == 0.0)
    {
        return 0.0;
    }
    else
    {
        double n = log10(abs(value));
        double m = pow(10.0, floor(n) - significantDigits + 1);
        return floor(value / m + 0.5) * m;
    }
}


static
QString readableNumber(double value, int significantDigits)
{
    double roundValue = value;
    int useDigits = 1;

    if (value != 0.0)
    {
        double n = log10(abs(value));
        useDigits = max(0, significantDigits - (int) n - 1);
        double m = pow(10.0, floor(n) - significantDigits + 1);
        roundValue = floor(value / m + 0.5) * m;
    }
    else
    {
        useDigits = significantDigits;
    }

    return QLocale::system().toString(roundValue, 'f', useDigits);
}


static
QString formatScientific(double v, int minExp = 4, int sigDigits = 4)
{
    if (v != 0.0)
    {
        int e = int(log10(fabs(v)));
        double m = v / pow(10.0, double(e));
        if (abs(e) >= minExp)
        {
            return QString("%1 &times; 10<sup>%2</sup>").arg(m).arg(e);
        }
        else
        {
            //return QString("%1").arg(v, 0, 'f');
            return readableNumber(v, sigDigits);
        }
    }
    else
    {
        return QString("0");
    }
}


static
QString formatDistance(double km)
{
    double au = ConvertDistance(km, Unit_Kilometer, Unit_AU);
    if (au > 0.1)
    {
        return QObject::tr("%1 AU (%2 km)").arg(au, 0, 'g', 4).arg(formatScientific(km, 7));
    }
    else
    {
        return QObject::tr("%1 km").arg(formatScientific(km, 7));
    }
}


QString
HelpCatalog::getObjectDataText(const QString &name) const
{
    // No help available; see if the named object has a custom info resource and use
    // that. If not, create a default info page.
    Entity* body = m_universeCatalog->find(name, Qt::CaseInsensitive);
    BodyInfo* info = NULL;
    if (body != NULL)
    {
        info = m_universeCatalog->findInfo(body);
    }

    QString text;
    QTextStream out(&text, QIODevice::WriteOnly);

    if (!body || !info)
    {
        // Object not found. Create an error page
        out << QString("<h1>%1</h1>No data available now.").arg(name);
    }
    else
    {
        out << QString("<h1>%1</h1>").arg(QString::fromUtf8(body->name().c_str()));
        double t = 0.0;

        if (body->geometry())
        {
            if (body->geometry()->isEllipsoidal())
            {
                Vector3d semiAxes = body->geometry()->ellipsoid().semiAxes();
                out << QString("Mean radius: %1 km<br>").arg(semiAxes.sum() / 3.0f);
            }
        }

        if (info->massKg > 0.0)
        {
            double earthMass = ConvertMass(info->massKg, Unit_Kilogram, Unit_EarthMass);
            QString kgMassString = QObject::tr("%1 kg").arg(formatScientific(info->massKg));

            if (earthMass > 0.001)
            {
                QString earthMassString;
                if (earthMass < 1.0)
                {
                    earthMassString = QObject::tr("%1% Earth").arg(earthMass * 100.0);
                }
                else
                {
                    earthMassString = QObject::tr("%1&times; Earth").arg(earthMass);
                }

                // Table form
                //out << QString("<table><tr><td>Mass</td><td>%1</td></tr> <tr><td></td><td>%2</td></tr></table>").arg(earthMassString).arg(kgMassString);

                // Single line form
                out << QObject::tr("Mass: %1 (%2)<br>").arg(earthMassString).arg(kgMassString);
            }
        }

        if (body->geometry() && body->geometry()->isEllipsoidal() && info->massKg > 0.0)
        {
            Vector3d semiAxes = body->geometry()->ellipsoid().semiAxes();
            double volumeKm3 = semiAxes.x() * semiAxes.y() * semiAxes.z() * 4.0 / 3.0 * PI;

            // Compute density in grams per cubic centimeter
            double rho = (info->massKg * 1000.0) / (volumeKm3 * 1.0e15);
            out << QObject::tr("Density: %1 g/cm<sup>3</sup><br>").arg(rho, 0, 'g', 3);
        }

        vesta::Arc* arc = body->chronology()->arc(0);
        if (arc)
        {
            Vector3d w = arc->rotationModel()->angularVelocity(t);
            double radPerSec = w.norm();
            if (radPerSec > 0.0)
            {
                double period = 360.0 / toDegrees(radPerSec);
                out << QString("Rotation period: %1<br>").arg(formatDuration(period));
            }
        }

        double sampleTime = body->chronology()->beginning() + body->chronology()->duration() / 2.0;

        // Compute Keplerian elements for objects with periodic orbits
        vesta::Arc* arc0 = body->chronology()->activeArc(sampleTime);

        if (arc0->trajectory()->isPeriodic() && arc0->center() != NULL)
        {
            BodyInfo* centerInfo = m_universeCatalog->findInfo(arc0->center());
            if (centerInfo && centerInfo->massKg > 0.0)
            {
                StateVector v = arc0->trajectory()->state(sampleTime);
                double M = info->massKg + centerInfo->massKg;
                double GM = 6.67384e-20 * M;

                double a = 1.0 / (2.0 / v.position().norm() - v.velocity().squaredNorm() / GM);

                //Vector3d L = v.position().cross(v.velocity());
                //double p = L.squaredNorm();
                double q = v.position().dot(v.velocity());
                Vector2d e(1.0 - v.position().norm() / a, q / sqrt(a * GM));

                double ecc = e.norm();

                QString apoapsisLabel = QObject::tr("Apoapsis");
                QString periapsisLabel = QObject::tr("Periapsis");
                if (arc0->center()->name() == "Sun")
                {
                    periapsisLabel = QObject::tr("Perihelion");
                    apoapsisLabel = QObject::tr("Aphelion");
                }
                else if (arc0->center()->name() == "Earth")
                {
                    periapsisLabel = QObject::tr("Perigee");
                    apoapsisLabel = QObject::tr("Apogee");
                }

                out << QObject::tr("<br><b>Orbit</b><br>");
                out << QString("Period: %1<br>").arg(formatDuration(arc0->trajectory()->period()));
                out << QString("%1: %2<br>").arg(periapsisLabel).arg(formatDistance(a * (1.0 - ecc)));
                out << QString("%1: %2<br>").arg(apoapsisLabel).arg(formatDistance(a * (1.0 + ecc)));
                out << QObject::tr("Semi-major axis: %1<br>").arg(formatDistance(a));
                out << QObject::tr("Eccentricity: %1<br>").arg(ecc, 0, 'g', 2);
            }
        }
    }

    return text;
}



/** Add (or replace) a help resource in the catalog.
  */
void
HelpCatalog::setHelpText(const QString& name, const QString& text)
{
    m_helpResources.insert(name, text);
}
