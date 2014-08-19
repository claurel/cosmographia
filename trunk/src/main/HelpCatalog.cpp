// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "HelpCatalog.h"
#include "UnitConversion.h"
#include "TleTrajectory.h"
#include "DateUtility.h"
#include "NumberFormat.h"
#include "catalog/UniverseCatalog.h"
#include "geometry/MeshInstanceGeometry.h"
#include <vesta/Geometry.h>
#include <vesta/Arc.h>
#include <vesta/Trajectory.h>
#include <vesta/RotationModel.h>
#include <vesta/Units.h>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <algorithm>
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;


static const double G = 6.67384e-20;
static const double EarthG = 9.80665;

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

            QString dataPageLink = QString("<a href=\"help:%1?data\">Properties</a><br>").arg(resourceName);
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

        if (hours >= 3)
        {
            return QObject::tr("%1h %2m %3s").arg(hours).arg(int(minutes)).arg(int(seconds));
        }
        else
        {
            minutes += hours * 60;
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
QString formatScientific(double v, int minExp = 4, unsigned int sigDigits = 4)
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
            return NumberFormat(sigDigits).toString(v);
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
    double miles = ConvertDistance(km, Unit_Kilometer, Unit_Mile);

    DistanceUnit unit = GetDefaultMeasurementSystem() == ImperialUnits ? Unit_Mile : Unit_Kilometer;

    if (au > 0.1)
    {
        if (unit == Unit_Mile)
            return QObject::tr("%1 AU (%2 mi)").arg(au, 0, 'g', 4).arg(formatScientific(miles, 7));
        else
            return QObject::tr("%1 AU (%2 km)").arg(au, 0, 'g', 4).arg(formatScientific(km, 7));
    }
    else
    {
        if (unit == Unit_Mile)
            return QObject::tr("%1 mi").arg(formatScientific(miles, 7));
        else
            return QObject::tr("%1 km").arg(formatScientific(km, 7));
    }
}


static
QString formatAngle(double radians)
{
    double deg = toDegrees(radians);
    {
        return QObject::tr("%1&deg;").arg(deg, 0, 'f', 1);
    }
}


static QString
tableRow(const QString& s1, const QString& s2)
{
    return QString("<tr><td align=\"right\">%1</td><td>&nbsp;&nbsp;&nbsp;</td><td align=\"left\"><font color=\"#ccccff\">%2</font></td></tr>\n").arg(s1).arg(s2);
}


static QString
readableDistance(double km, unsigned int precision)
{
    if (GetDefaultMeasurementSystem() == ImperialUnits)
    {
        double miles = ConvertDistance(km, Unit_Kilometer, Unit_Mile);
        if (miles < 0.5)
        {
            double feet = ConvertDistance(km, Unit_Kilometer, Unit_Foot);
            return QString(QObject::tr("%1 feet").arg(NumberFormat(precision).toString(feet)));
        }
        else
        {
            return QString(QObject::tr("%1 miles").arg(NumberFormat(precision).toString(miles)));
        }
    }
    else
    {
        if (km < 0.5)
        {
            double meters = ConvertDistance(km, Unit_Kilometer, Unit_Meter);
            return QString(QObject::tr("%1 m").arg(NumberFormat(precision).toString(meters)));
        }
        else
        {
            return QString(QObject::tr("%1 km").arg(NumberFormat(precision).toString(km)));
        }
    }
}


QString
HelpCatalog::getObjectDataText(const QString &name) const
{
    bool isEarthSat = false;

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

    out << "<html><head>\n";
    out << "<style type=\"text/css\"> a { color: #72c0ff }</style>\n";
    out << "</head><body>";

    if (!body || !info)
    {
        // Object not found. Create an error page
        out << QString("<h1>%1</h1>No data available now.").arg(name);
    }
    else
    {
        // Osculating elements will be calculated at this time
        double sampleTime = body->chronology()->beginning() + body->chronology()->duration() / 2.0;

        // Special case for TLE orbits: use the current system time instead
        if (body->chronology()->arcCount() > 0)
        {
            TleTrajectory* tle = dynamic_cast<TleTrajectory*>(body->chronology()->firstArc()->trajectory());
            if (tle)
            {
                vesta::GregorianDate now = QtDateToVestaDate(QDateTime::currentDateTimeUtc());
                sampleTime = now.toTDBSec();
                isEarthSat = true;
            }
        }

        // Earth needs a bit of special handling, such as eliding the 'percent of Earth' comparisons
        bool isEarth = body->name() == "Earth";

        out << QString("<h1>%1</h1>").arg(QString::fromUtf8(body->name().c_str()));

        out << "<br>";
        out << "<table>\n";

        out << tableRow(QObject::tr("<b>Physical</b>"), "");
        out << tableRow("", "");

        if (body->geometry())
        {
            if (body->geometry()->isEllipsoidal())
            {
                Vector3d semiAxes = body->geometry()->ellipsoid().semiAxes();
                bool isSpherical = semiAxes.x() == semiAxes.y() && semiAxes.y() == semiAxes.z();
                if (isSpherical)
                {
                    //out << tableRow(QObject::tr("Radius"), QObject::tr("%1 km").arg(NumberFormat(4u).toString(semiAxes.x())));
                    out << tableRow(QObject::tr("Radius"), readableDistance(semiAxes.x(), 4u));
                }
                else
                {
                    //out << tableRow(QObject::tr("Mean radius"), QObject::tr("%1 km").arg(NumberFormat(4u).toString(semiAxes.sum() / 3.0f)));
                    //out << tableRow(QObject::tr("Equatorial radius"), QObject::tr("%1 km").arg(NumberFormat(4u).toString((semiAxes.x() + semiAxes.y()) / 2.0f)));
                    //out << tableRow(QObject::tr("Polar radius"), QObject::tr("%1 km").arg(NumberFormat(4u).toString(semiAxes.z())));
                    out << tableRow(QObject::tr("Equatorial radius"), readableDistance((semiAxes.x() + semiAxes.y()) / 2.0f, 4u));
                    out << tableRow(QObject::tr("Polar radius"), readableDistance(semiAxes.z(), 4u));
                }
            }
            else if (dynamic_cast<MeshInstanceGeometry*>(body->geometry()))
            {
                BoundingBox bbox = dynamic_cast<MeshInstanceGeometry*>(body->geometry())->boundingBox();
                Vector3f extents = bbox.extents();
                extents = Vector3f(roundToSigDigits(extents.x(), 3),
                                   roundToSigDigits(extents.y(), 3),
                                   roundToSigDigits(extents.z(), 3));

                out << tableRow(QObject::tr("Dimensions"), QObject::tr("%1 &times; %2 &times; %3 km").arg(extents.x()).arg(extents.y()).arg(extents.z()));
            }
        }

        if (info->massKg > 0.0)
        {
            double earthMass = ConvertMass(info->massKg, Unit_Kilogram, Unit_EarthMass);

            QString massString = QObject::tr("%1 kg").arg(formatScientific(info->massKg));
            if (GetDefaultMeasurementSystem() == ImperialUnits)
            {
                massString = QObject::tr("%1 tons").arg(formatScientific(ConvertMass(info->massKg, Unit_Kilogram, Unit_Ton)));
            }

            if (earthMass > 0.001)
            {
                QString earthMassString;
                if (earthMass < 1.0)
                {
                    earthMassString = QObject::tr("%1% Earth").arg(roundToSigDigits(earthMass * 100.0, 2));
                }
                else
                {
                    earthMassString = QObject::tr("%1&times; Earth").arg(earthMass);
                }

                // Single line form
                out << tableRow(QObject::tr("Mass"), QString("%1 (%2)").arg(earthMassString).arg(massString));
            }
            else
            {
                out << tableRow(QObject::tr("Mass"), massString);
            }
        }

        double rho = info->density;
        if (rho == 0.0)
        {
            // Automatically calculate the density
            if (body->geometry() && body->geometry()->isEllipsoidal() && info->massKg > 0.0)
            {
                Vector3d semiAxes = body->geometry()->ellipsoid().semiAxes();
                double volumeKm3 = semiAxes.x() * semiAxes.y() * semiAxes.z() * 4.0 / 3.0 * PI;

                // Compute density in grams per cubic centimeter
                rho = (info->massKg * 1000.0) / (volumeKm3 * 1.0e15);
            }
        }

        if (rho > 0.0)
        {
            out << tableRow(QObject::tr("Density"), QObject::tr("%1 g/cm<sup>3</sup>").arg(rho, 0, 'g', 3));

            // Only show surface gravity for ellipsoidal objects. For irregular bodies, it varies
            // dramatically over the surface
            if (body->geometry() && body->geometry()->isEllipsoidal() && info->massKg > 0.0)
            {
                Vector3d semiAxes = body->geometry()->ellipsoid().semiAxes();

                double equatorialRadius = (semiAxes.x() + semiAxes.y()) / 2.0;
                double surfaceGravity = (G * info->massKg) / pow(equatorialRadius, 2.0) * 1000;
                double earthGravPercent = surfaceGravity / EarthG * 100;
                if (isEarth)
                {
                    // Force the displayed value to standard gravity for Earth. The calculated
                    // value will be off by a small amount.
                    surfaceGravity = EarthG;
                }

                QString units = QObject::tr("m/s");
                if (GetDefaultMeasurementSystem() == ImperialUnits)
                {
                    surfaceGravity = ConvertDistance(surfaceGravity, Unit_Meter, Unit_Foot);
                    units = QObject::tr("ft/s");
                }

                if (isEarth)
                {
                    // Omit the % of Earth display for Earth
                    out << tableRow(QObject::tr("Surface gravity"), QObject::tr("%1 %2<sup>2</sup>").
                                    arg(roundToSigDigits(surfaceGravity, 3)).arg(units));
                }
                else
                {
                    out << tableRow(QObject::tr("Surface gravity"), QObject::tr("%1% Earth (%2 %3<sup>2</sup>)").
                                    arg(roundToSigDigits(earthGravPercent, 3)).
                                    arg(roundToSigDigits(surfaceGravity, 3)).
                                    arg(units));
                }
            }
        }

        vesta::Arc* arc = body->chronology()->activeArc(sampleTime);

        double orbitalPeriod = 0.0;
        if (arc->trajectory()->isPeriodic())
        {
            orbitalPeriod = arc->trajectory()->period();
        }

        if (arc)
        {
            Vector3d w = arc->rotationModel()->angularVelocity(sampleTime);
            double radPerSec = w.norm();
            if (radPerSec > 0.0)
            {
                double period = 360.0 / toDegrees(radPerSec);

                // If the orbital period and rotation period are significantly close,
                // indicate that the object is a synchronous rotator.
                QString syncNotice;
                if (abs(orbitalPeriod / period - 1.0) < 1.0e-3)
                {
                    syncNotice = QObject::tr(" (synchronous)");
                }

                //out << tableRow(QObject::tr("<b>Rotation</b><font size=\"-3\"><br>&nbsp;</font>"), "");
                out << tableRow("", "");
                out << tableRow(QObject::tr("Rotation period"), QString("%1 %2").arg(formatDuration(period)).arg(syncNotice));
            }
        }

        out << tableRow("", "");
        out << tableRow("", "");

        // Compute Keplerian elements for objects with periodic orbits
        if (arc->trajectory()->isPeriodic() && arc->center() != NULL)
        {
            BodyInfo* centerInfo = m_universeCatalog->findInfo(arc->center());
            if (centerInfo && centerInfo->massKg > 0.0)
            {
                StateVector v = arc->trajectory()->state(sampleTime);
                double M = info->massKg + centerInfo->massKg;
                double GM = 6.67384e-20 * M;

                double a = 1.0 / (2.0 / v.position().norm() - v.velocity().squaredNorm() / GM);

                Vector3d N = v.position().normalized().cross(v.velocity().normalized());
                double inclination = acos(N.z());

                //Vector3d L = v.position().cross(v.velocity());
                //double p = L.squaredNorm();
                double q = v.position().dot(v.velocity());
                Vector2d e(1.0 - v.position().norm() / a, q / sqrt(a * GM));

                double ecc = e.norm();

                QString apoapsisLabel = QObject::tr("Apoapsis");
                QString periapsisLabel = QObject::tr("Periapsis");
                if (arc->center()->name() == "Sun")
                {
                    periapsisLabel = QObject::tr("Perihelion");
                    apoapsisLabel = QObject::tr("Aphelion");
                }
                else if (arc->center()->name() == "Earth")
                {
                    periapsisLabel = QObject::tr("Perigee");
                    apoapsisLabel = QObject::tr("Apogee");
                }

                double periapsis = a * (1.0 - ecc);
                double apoapsis = a * (1.0 + ecc);
                if (isEarthSat)
                {
                    periapsis -= 6378.1;
                    apoapsis -= 6378.1;
                }

                out << tableRow(QObject::tr("<b>Orbit</b>"), "");
                out << tableRow("", "");
                out << tableRow("Period", formatDuration(orbitalPeriod));
                out << tableRow(periapsisLabel, formatDistance(periapsis));
                out << tableRow(apoapsisLabel, formatDistance(apoapsis));
                out << tableRow(QObject::tr("Semi-major axis"), formatDistance(a));
                out << tableRow(QObject::tr("Eccentricity"), QString::number(ecc, 'g', 2));
                if (isEarthSat)
                {
                    out << tableRow(QObject::tr("Inclination"), formatAngle(inclination));
                }
            }
        }

        out << "</table>\n";

        out << "<br><br><br><a href=\"help:properties\">" << QObject::tr("Explain this data...") << "</a>";
    }

    out << "</body></html>";

    return text;
}



/** Add (or replace) a help resource in the catalog.
  */
void
HelpCatalog::setHelpText(const QString& name, const QString& text)
{
    m_helpResources.insert(name, text);
}
