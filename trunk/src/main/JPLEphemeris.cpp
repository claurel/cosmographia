// JPLEphemeris.h
//
// Copyright (C) 2010-2013 Chris Laurel <claurel@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met: 
//
//    1. Redistributions of source code must retain the above copyright notice, this
//       list of conditions and the following disclaimer. 
//    2. Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "JPLEphemeris.h"
#include <vesta/Units.h>
#include <QDataStream>
#include <QFile>
#include <QDebug>

using namespace vesta;
using namespace std;


JPLEphemeris::JPLEphemeris() :
    m_earthMoonMassRatio(0.0)
{
}


JPLEphemeris::~JPLEphemeris()
{
}


ChebyshevPolyTrajectory*
JPLEphemeris::trajectory(JplObjectId id) const
{
    if ((int) id >= (int) ObjectCount)
    {
        return NULL;
    }
    else
    {
        return m_trajectories[(int) id].ptr();
    }
}


void
JPLEphemeris::setTrajectory(JplObjectId id, ChebyshevPolyTrajectory* trajectory)
{
    if (int(id) < int(ObjectCount))
    {
        m_trajectories[int(id)] = trajectory;
    }
}


struct JplEphCoeffInfo
{
    quint32 offset;
    quint32 coeffCount;
    quint32 granuleCount;
};


JPLEphemeris*
JPLEphemeris::load(const string& filename)
{
    const int JplEph_LabelSize                      =   84;
    const unsigned int JplEph_ConstantCount         =  400;
    const unsigned int JplEph_ConstantNameLength    =    6;
    const unsigned int JplEph_ObjectCount           =   12; // Sun, Moon, planets (incl. Pluto), Earth-Moon bary.

    const unsigned int DE406_RecordSize             =  728;

    QFile ephemFile(filename.c_str());
    if (!ephemFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Ephemeris file is missing!";
        return NULL;
    }

    QDataStream in(&ephemFile);
    in.setByteOrder(QDataStream::BigEndian);

    in.skipRawData(JplEph_LabelSize * 3);
    if (in.status() != QDataStream::Ok)
    {
        return NULL;
    }

    // Skip past the constant names
    in.skipRawData(JplEph_ConstantCount * JplEph_ConstantNameLength);
    if (in.status() != QDataStream::Ok)
    {
        return NULL;
    }

    double startJd = 0.0;
    double endJd = 0.0;
    double daysPerRecord = 0.0;
    in >> startJd;
    in >> endJd;
    in >> daysPerRecord;
    if (in.status() != QDataStream::Ok)
    {
        return NULL;
    }

    // Skip number of constants with values
    in.skipRawData(sizeof(quint32));

    double kmPerAu = 0.0;
    in >> kmPerAu;
    double earthMoonMassRatio = 0.0;
    in >> earthMoonMassRatio;

    JplEphCoeffInfo coeffInfo[JplEph_ObjectCount];
    for (unsigned int objectIndex = 0; objectIndex < JplEph_ObjectCount; ++objectIndex)
    {
        in >> coeffInfo[objectIndex].offset;
        in >> coeffInfo[objectIndex].coeffCount;
        in >> coeffInfo[objectIndex].granuleCount;

        if (in.status() != QDataStream::Ok)
        {
            return NULL;
        }

        // Convert to a zero-based offset
        coeffInfo[objectIndex].offset--;
    }

    qint32 ephemNumber = 0;
    in >> ephemNumber;
    if (ephemNumber != 406)
    {
        return NULL;
    }

    // Skip libration information (offset, coeff. count, granule count)
    in.skipRawData(sizeof(quint32) * 3);
    if (in.status() != QDataStream::Ok)
    {
        return NULL;
    }

    // Skip the rest of the record
    in.skipRawData(DE406_RecordSize * 8 - 2856);
    // Skip the constants
    in.skipRawData(DE406_RecordSize * 8);
    if (in.status() != QDataStream::Ok)
    {
        return NULL;
    }

    unsigned int recordCount = (unsigned int) ((endJd - startJd) / daysPerRecord);

    vector<double> objectCoeffs[JplEph_ObjectCount];

    for (unsigned int i = 0; i < recordCount; ++i)
    {
        double startTime = 0.0;
        double endTime = 0.0;
        in >> startTime >> endTime;

        for (unsigned int objectIndex = 0; objectIndex < JplEph_ObjectCount; ++objectIndex)
        {
            unsigned int coeffCount = coeffInfo[objectIndex].coeffCount * coeffInfo[objectIndex].granuleCount;
            if (coeffCount != 0)
            {
                for (unsigned int j = 0; j < coeffCount; ++j)
                {
                    double x = 0.0;
                    double y = 0.0;
                    double z = 0.0;
                    in >> x >> y >> z;
                    objectCoeffs[objectIndex].push_back(x);
                    objectCoeffs[objectIndex].push_back(y);
                    objectCoeffs[objectIndex].push_back(z);
                }
            }
        }

        // Make sure that we read this record successfully
        if (in.status() != QDataStream::Ok)
        {
            return NULL;
        }
    }

    JPLEphemeris* eph = new JPLEphemeris;
    double startSec = daysToSeconds(startJd - vesta::J2000);
    double secsPerRecord = daysToSeconds(daysPerRecord);

    const double orbitalPeriods[] =
    {
        0.24085, 0.61520, 1.0000, 1.8808, 11.863, 29.447, 84.017, 164.79, 248.02,
        27.32158 / 365.25, // Moon, about Earth-Moon barycenter
        0.0,
        27.32158 / 365.25 // Earth, about Earth-Moon barycenter
    };

    for (unsigned int objectIndex = 0; objectIndex < JplEph_ObjectCount - 1; ++objectIndex)
    {
        ChebyshevPolyTrajectory* trajectory =
                new ChebyshevPolyTrajectory(&objectCoeffs[objectIndex][0],
                                            coeffInfo[objectIndex].coeffCount - 1,
                                            coeffInfo[objectIndex].granuleCount * recordCount,
                                            startSec,
                                            secsPerRecord / coeffInfo[objectIndex].granuleCount);
        trajectory->setPeriod(daysToSeconds(orbitalPeriods[objectIndex] * 365.25));
        eph->setTrajectory(JplObjectId(objectIndex), trajectory);
    }

    // Set constants
    eph->m_earthMoonMassRatio = earthMoonMassRatio;

    return eph;
}
