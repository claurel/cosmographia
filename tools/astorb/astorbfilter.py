#!/usr/bin/python

# Filter an asteroid orbit catalog in the ASCII format described here:
#  http://www.naic.edu/~nolan/astorb.html
#
# Only minor planets with semi-major axes in the specified range will
# be written out.
#
# Command line:
#
# astorbfilter.py <input catalog> <min. SMA> <max. SMA>
#
# - SMAs are given in AU
# - Matching records are written to standard output
#
# Example:
#
# astorbfilter.py astorb.dat 0.0 1.0
#
# This line will filter out any asteroids with a semi-major axes greater
# than 1 AU

import struct
import sys
import datetime
import struct
import re

astorb = open(sys.argv[1], 'r')
minSMA = float(sys.argv[2])
maxSMA = float(sys.argv[3])

j2000 = datetime.datetime(2000, 1, 1, 12, 0, 0)

for record in astorb:
    id = record[7:26].strip()

    meanAnomaly = float(record[115:125])
    argOfPeriapsis = float(record[126:136])
    ascendingNode = float(record[137:147])
    inclination = float(record[148:157])
    eccentricity = float(record[158:168])
    sma = float(record[169:181])

    if sma >= minSMA and sma <= maxSMA:
        sys.stdout.write(record)


    

    
