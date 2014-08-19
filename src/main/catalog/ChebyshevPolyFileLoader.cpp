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

#include "ChebyshevPolyFileLoader.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

using namespace vesta;

static const char* ChebyshevPolyFileHeader = "CHEBPOLY";


/** Load a binary file containing an orbit represented as an array of Chebyshev
  * polynomials.
  *
  * The file has the following format:
  *
  * 8 bytes - header "CHEBPOLY"
  * 4 bytes - int32 - record count
  * 4 bytes - int32 - polynomial degree
  * 8 bytes - double - start time (seconds since J2000.0 TDB)
  * 8 bytes - double - interval covered by each polynomial (in seconds)
  * data - list of doubles, count = 3 * (degree + 1) * record count
  *
  * Polynomial coefficients for each interval are stored as:
  *   x0 x1 x2 ... xn y0 y1 y2 ... yn z0 z1 z2 ... zn
  *
  * Byte order is little endian (Intel x86)
  */
ChebyshevPolyTrajectory*
LoadChebyshevPolyFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Unable to open Chebyshev polynomial trajectory file " << fileName;
        return NULL;
    }

    QDataStream in(&file);

    // All floating point values in the file are double precision
    in.setVersion(QDataStream::Qt_4_7);
    in.setFloatingPointPrecision(QDataStream::DoublePrecision);
    in.setByteOrder(QDataStream::LittleEndian);

    char header[8];
    in.readRawData(header, sizeof(header));
    if (QString::fromLatin1(header, sizeof(header)) != ChebyshevPolyFileHeader)
    {
        qDebug() << "File " << fileName << " is not a Chebyshev polynomial trajectory file.";
        return NULL;
    }

    quint32 recordCount   = 0;
    quint32 degree        = 0;
    double startTime      = 0.0;
    double intervalLength = 0.0;

    in >> recordCount >> degree >> startTime >> intervalLength;
    if (in.status() != QDataStream::Ok)
    {
        qDebug() << "Error reading header from Chebyshev polynomial file " << fileName;
        return NULL;
    }

#if 0
    qDebug() << "Chebyshev file " << fileName << ": "
             << recordCount << " records, "
             << " degree " << degree
             << ", interval " << intervalLength / 86400.0 << " days";
#endif

    unsigned int recordSize = 3 * (degree + 1);
    unsigned int coeffCount = recordSize * recordCount;

    double* coeffs = new double[coeffCount];
    if (!coeffs)
    {
        qDebug() << "Not enough memory to load Chebyshev polynomial trajectory " << fileName;
        return NULL;
    }

    if (in.readRawData(reinterpret_cast<char*>(coeffs), coeffCount * sizeof(double)) < 0)
    {
        qDebug() << "Error reading coefficients from Chebyshev polynomial file " << fileName;
        delete[] coeffs;
        return NULL;
    }

    // Change layout from structure of arrays to array of structures
    /* Not currently required
    double* swapBuffer = new double[recordSize];
    for (unsigned int rec = 0; rec < recordCount; ++rec)
    {
        for (unsigned int i = 0; i <= degree; ++i)
        {
            for (unsigned int j = 0; j < 3; ++j)
            {
                swapBuffer[i * 3 + j] = coeffs[rec * recordSize + j * (degree + 1) + i];//coeffs[rec * recordSize + i * (degree + 1) + j];
            }
        }
        qCopy(swapBuffer, swapBuffer + recordSize, coeffs + rec * recordSize);
    }
    delete[] swapBuffer;
    */

    ChebyshevPolyTrajectory* trajectory = new ChebyshevPolyTrajectory(coeffs, degree, recordCount, startTime, intervalLength);
    delete[] coeffs;

    return trajectory;
}
