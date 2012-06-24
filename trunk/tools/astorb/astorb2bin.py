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
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-o", "--out", dest="outfile",
                  help="Write binary catalog to FILE", metavar="FILE")
parser.add_option("-i", "--in", dest="infile",
                  help="Read asteroid orbital data from FILE", metavar="FILE")
parser.add_option("-d", "--discovery", dest="discfilename",
                  help="Read discovery dates from FILE", metavar="FILE")

(options, args) = parser.parse_args()

astorb = sys.stdin
if options.infile:
    astorb = open(options.infile, 'r')

out = sys.stdout
if options.outfile:
    out = open(options.outfile, 'wb')

discdates = {}
if options.discfilename:
    discfile = open(options.discfilename, 'r')
    for record in discfile:
        num = int(record[0:8].strip('() '))
        year = record[41:45]
        month = record[46:48]
        day = record[49:51]
        date = datetime.datetime(int(year), int(month), int(day), 12, 0, 0)
        discdates[num] = date
        
j2000 = datetime.datetime(2000, 1, 1, 12, 0, 0)
provisionalDesignation = re.compile('[0-9]{4,4} [A-Z]{2,2}[0-9]*')

for record in astorb:
    num = record[0:6].strip()
    if num:
        num = int(num)
    else:
        num = 0

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

    discoveryDate = discdates.get(num, False)
    if discoveryDate:
        # print num, id, discoveryDate
        dt = (discoveryDate - j2000)
        discoveryDate = 2451545.0 + dt.days + dt.seconds / 86400.0
    else:
        # Extract the approximate discovery date from the provision designation
        discoveryDate = 0
        if provisionalDesignation.match(id):
            month = (ord(id[5]) - 65) / 2 + 1
            discoveryDate = 2451545 + (float(id[0:4]) - 2000) * 365.25 + month * 30.0
        elif id.endswith("P-L"):
            # Palomar-Leiden survey, 1960
            discoveryDate = 2451545 + (1960 - 2000) * 365.25
        elif id.endswith("T-1"):
            # First Trojan survey, 1971
            discoveryDate = 2451545 + (1971 - 2000) * 365.25
        elif id.endswith("T-2"):
            # Second Trojan survey, 1973
            discoveryDate = 2451545 + (1973 - 2000) * 365.25
        elif id.endswith("T-3"):
            # Third Trojan survey, 1977
            discoveryDate = 2451545 + (1977 - 2000) * 365.25
        else:
            print "Bad identifier: ", id
    
    binrecord = struct.pack('>ffffffdf', sma, eccentricity, inclination, ascendingNode, argOfPeriapsis, meanAnomaly, epoch, discoveryDate)
    out.write(binrecord)
    

    
