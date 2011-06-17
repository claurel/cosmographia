#!/usr/bin/python

# Convert an asteroid orbit catalog in the ASCII format described here:
#  http://www.naic.edu/~nolan/astorb.html
#
# ...into a more compact binary format for use in Cosmographia. The orbital
# elements are written out as single-precision floating point values. Such
# low precision is still useful for visualization of the dynamics of large
# populations of asteroids.

import struct
import sys
import datetime
import struct
import re

astorb = open(sys.argv[1], 'r')
out = open(sys.argv[2], 'wb')

j2000 = datetime.datetime(2000, 1, 1, 12, 0, 0)
provisionalDesignation = re.compile('[0-9]{4,4} [A-Z]{2,2}[0-9]*')

for record in astorb:
    id = record[7:26].strip()
    epochYear = record[106:110]
    epochMonth = record[110:112]
    epochDay = record[112:114]
    date = datetime.datetime(int(epochYear), int(epochMonth), int(epochDay), 12, 0, 0)
    dt = (date - j2000)

    meanAnomaly = float(record[115:125])
    argOfPeriapsis = float(record[126:136])
    ascendingNode = float(record[137:147])
    inclination = float(record[148:157])
    eccentricity = float(record[158:168])
    sma = float(record[169:181])

    epoch = 2451545.0 + dt.days + dt.seconds / 86400.0

    discoveryDate = 0

    # Extract the approximate discovery date from the provision designation
    if provisionalDesignation.match(id):
        month = (ord(id[5]) - 65) / 2 + 1
        discoveryDate = 2451545 + (float(id[0:4]) - 2000) * 365.25 + month * 30.0

    binrecord = struct.pack('>ffffffdf', sma, eccentricity, inclination, ascendingNode, argOfPeriapsis, meanAnomaly, epoch, discoveryDate)
    out.write(binrecord)
    

    
