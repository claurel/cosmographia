/*
 * Copyright (C) 2011 by Chris Laurel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 */

/** spkx - Extract Chebyshev polynomial coefficients from a SPICE SPK file
 *
 * The binary output file has the following format:
 *
 * 8 bytes - header "CHEBPOLY"
 * 4 bytes - int32 - record count
 * 4 bytes - int32 - polynomial degree
 * 8 bytes - double - start time (seconds since J2000.0 TDB)
 * 8 bytes - double - interval covered by each polynomial (in seconds)
 * data - 3 * sizeof(double) * (degree + 1) * record count bytes
 *
 * Polynomial coefficients for each interval are stored as:
 *   x0 x1 x2 ... xn y0 y1 y2 ... yn z0 z1 z2 ... zn
 *
 * Byte order is little endian (Intel x86)
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include "SpiceUsr.h"
#include "SpiceZfc.h"

using namespace std;

typedef int int32;
typedef unsigned int uint32;


// Extract just the positions from SPK Type 3 data (Chebyshev polynomials
// for position and velocity.)
double*
extractXyzCoeffs(const double* coeffs, int degree, int recordCount)
{
    int oldRecordSize = (degree + 1) * 6 + 2;
    int newRecordSize = (degree + 1) * 3;
    double* xyzCoeffs = new double[newRecordSize * recordCount];

    const double* record = coeffs;
    for (int i = 0; i < recordCount; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            copy(record + 2 + (degree + 1) * j,
                 record + 2 + (degree + 1) * (j + 1),
                 xyzCoeffs + i * newRecordSize + (degree + 1) * j);
        }
        record += oldRecordSize;
    }

    return xyzCoeffs;
}


// Extract just the positions from SPK Type 3 data (Chebyshev polynomials
// for position and velocity.)
void
extractType3XyzCoeffs(const double* coeffs,
                      double* xyzCoeffs,
                      int degree)
{
    int n = degree + 1;

    for (int i = 0; i < 3; ++i)
    {
        copy(coeffs + 2 + n * i, coeffs + 2 + n * (i + 1), xyzCoeffs + n * i);
    }
}


// Extract just the positions from SPK Type 2 data (Chebyshev polynomials
// for position.)
void
extractType2XyzCoeffs(const double* coeffs,
                      double* xyzCoeffs,
                      int degree)
{
    int n = degree + 1;
    copy(coeffs + 2, coeffs + 2 + n * 3, xyzCoeffs);
}


string naifName(SpiceInt code)
{
    char buffer[128];
    SpiceBoolean found = 0;
    bodc2n_c(code, sizeof(buffer), buffer, &found);
    if (found != 0)
    {
        return string(buffer);
    }
    else
    {
        return "UNKNOWN";
    }
}


string frameName(SpiceInt frameCode)
{
    char buffer[128];
    frmnam_c(frameCode, sizeof(buffer), buffer);
    if (iswhsp_c(buffer))
    {
        return "UNKNOWN";
    }
    else
    {
        return string(buffer);
    }
}


int main(int argc, char* argv[])
{
    if (argc != 6)
    {
        cerr << "Usage: <spk file> <output file> <start time> <end time> <body name>\n";
        return 1;
    }

    furnsh_c("naif0009.tls");

    const char* fileName = argv[1];
    const char* outputFile = argv[2];
    const char* startTimeStr = argv[3];
    const char* endTimeStr = argv[4];
    const char* targetName = argv[5];

    SpiceInt targetId = 0;
    SpiceBoolean found = SPICEFALSE;

    bodn2c_c(targetName, &targetId, &found);
    if (found == SPICEFALSE)
    {
        cerr << "Unknown body name " << targetName << endl;
        return 1;
    }

    double startET = 0.0;
    double endET = 0.0;
    str2et_c(startTimeStr, &startET);
    str2et_c(endTimeStr, &endET);

    cout << startET << ", " << endET << endl;

    cout << "Extracting Chebyshev coefficients for " << targetName << " (NAIF Code: " << targetId << ")\n";

    SpiceInt spk;

    dafopr_c(fileName, &spk);

    int count = 0;
    dafbfs_c(spk);
    daffna_c(&found);

    while (found)
    {
        SpiceDouble summary[128];

        const SpiceInt ND = 2;
        const SpiceInt NI = 6;
        SpiceDouble sd[ND];
        SpiceInt si[NI];

        dafgs_c(summary);
        dafus_c(summary, ND, NI, sd, si);

        double spkStartET = sd[0];
        double spkEndET = sd[1];
        
        SpiceInt id = si[0];
        SpiceInt centerId = si[1];
        SpiceInt ref = si[2];
        SpiceInt dataType = si[3];
        SpiceInt begin = si[4];
        SpiceInt end = si[5];

        cout.precision(16);
        cout << naifName(id) << ", Center: " << naifName(centerId) << ", Frame: " << frameName(ref) << ", Type: " << dataType << endl;

        if (id == targetId)
        {
            if (startET < spkStartET || endET > spkEndET)
            {
                cerr << "SPK coverage for target does not completely include requested range.\n";
                return 1;
            }

            if (dataType != 2 && dataType != 3)
            {
                cerr << "Target found, but SPK data type is wrong (not Chebyshev polynomials.\n";
                return 1;
            }

            {
                double descRecord[4];
                SpiceInt descBegin = end - 3;
                dafgda_(&spk, &descBegin, &end, descRecord);

                double initialET = descRecord[0];
                double interval = descRecord[1];
                SpiceInt recordSize = SpiceInt(descRecord[2]);
                SpiceInt recordCount = SpiceInt(descRecord[3]);
                int components = dataType == 2 ? 3 : 6;
                int32 degree = (recordSize - 2) / components - 1;

                cout << "  ET: " << initialET 
                     << ", interval: " << interval / 86400.0 << "d"
                     << ", size: " << recordSize
                     << ", degree: " << degree
                     << ", count: " << recordCount << endl;
                
                int32 beginOutRecord = int((startET - spkStartET) / interval);
                int32 endOutRecord = int(ceil(endET - spkStartET) / interval);
                int32 outRecordCount = endOutRecord - beginOutRecord + 1;
                double outInitialET = initialET + interval * beginOutRecord;

                int totalSize = (degree + 1) * 3 * sizeof(double) * outRecordCount;
                cout << "Writing " << outRecordCount
                     << " records, size " << ((totalSize / (1024.0 * 1024.0))) << " MB" << endl;

                ofstream out(outputFile, ios::out | ios::binary);
                const char* header = "CHEBPOLY";
                out.write(header, 8);
                out.write((char*) &outRecordCount, sizeof(outRecordCount));
                out.write((char*) &degree, sizeof(degree));
                out.write((char*) &outInitialET, sizeof(outInitialET));
                out.write((char*) &interval, sizeof(interval));

                int xyzCoeffCount = (degree + 1) * 3;
                double* coeffs = new double[recordSize];
                double* xyzCoeffs = new double[xyzCoeffCount];

                for (int rec = beginOutRecord; rec <= endOutRecord; ++rec)
                {
                    SpiceInt recBegin = begin + recordSize * rec;
                    SpiceInt recEnd = recBegin + recordSize - 1;
                    dafgda_(&spk, &recBegin, &recEnd, coeffs);

                    if (dataType == 2)
                    {
                        extractType2XyzCoeffs(coeffs, xyzCoeffs, degree);
                    }
                    else 
                    {
                        extractType3XyzCoeffs(coeffs, xyzCoeffs, degree);
                    }

                    out.write((char*) xyzCoeffs, sizeof(double) * xyzCoeffCount);

                    if (rec < beginOutRecord + 1 && false)
                    {
                        for (int i = 0; i <= degree; ++i)
                        {
                            cout << xyzCoeffs[i] << ", "
                                 << xyzCoeffs[(degree + 1) + i] << ", "
                                 << xyzCoeffs[(degree + 1) * 2 + i] << endl;
                        }
                        cout << endl;
                    }
                }

                delete[] coeffs;
                delete[] xyzCoeffs;
            }
        }

        ++count;
        daffna_c(&found);
    }

    cout << "Segment count: " << count << endl;

    return 0;
}
