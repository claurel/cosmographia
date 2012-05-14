#!/usr/bin/python

import struct
import sys
import math
import datetime
import struct
import re

tlefile = open(sys.argv[1], 'r')
out = open(sys.argv[2], 'wb')

muEarth = 3.986004418e5


def outputSat(line1, line2):
    inclination = float(line2[8:16])
    ascendingNode = float(line2[17:25])
    eccentricity = float("0." + line2[26:33])
    argOfPeriapsis = float(line2[34:42])
    meanAnomaly = float(line2[43:51])
    revsPerDay = float(line2[52:63])
    meanMotion = 360.0 * revsPerDay
    day = float(line1[20:32])
    year = int(line1[18:20])
    if year < 57:
        year = year + 2000
    else:
        year = year + 1900

    epochDate = datetime.datetime(year, 1, 1) + datetime.timedelta(day - 1)
    dt = epochDate - datetime.datetime(2000, 1, 1)
    epoch = 2451544.5 + dt.days + dt.seconds / 86400.0

    periodSec = 86400 / revsPerDay
    # Derive the semimajor axis from the mean motion
    sma = math.pow((muEarth * math.pow(periodSec, 2.0)) / (4.0 * math.pi * math.pi), 1.0 / 3.0);
    print sma, periodSec / 60, epochDate, epoch

    binrecord = struct.pack('>fffffffd', sma, eccentricity, inclination, ascendingNode, argOfPeriapsis, meanAnomaly, meanMotion, epoch)
    out.write(binrecord)


done = False
while not done:
    satId = tlefile.readline()
    line1 = tlefile.readline()
    line2 = tlefile.readline()
    if len(satId) > 0:
        satId = satId.rstrip('\n\r ')
        line1 = line1.rstrip('\n\r')
        line2 = line2.rstrip('\n\r')
        print satId
        outputSat(line1, line2)
    else:
        done = True

    
