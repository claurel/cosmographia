#!/usr/bin/python

import sys
import datetime
import json
import re
import math

astorb = open(sys.argv[1], 'r')

magThreshold = 7.5

j2000 = datetime.datetime(2000, 1, 1, 12, 0, 0)
provisionalDesignation = re.compile('[0-9]{4,4} [A-Z]{2,2}[0-9]*')

for record in astorb:
    id = record[7:26].strip()
    name = id
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
    period = math.pow(sma, 1.5)

    radius = 1
    diameter = record[59:65].strip()
    if diameter != "":
        radius = float(diameter) / 2.0

    epoch = 2451545.0 + dt.days + dt.seconds / 86400.0
    epochString = '{0}-{1}-{2}T00:00:00'.format(epochYear, epochMonth, epochDay)

    model = 'models/hyperion.cmod'

    t = { 'name' : name,
          'class' : 'asteroid',
          'label' : { 'color' : '#aa8855' },
          'trajectoryPlot' : { 'fade' : 0.3 },
          'center' : 'Sun',
          'trajectoryFrame' : 'EclipticJ2000',
          'trajectory' : {
            'type' : 'Keplerian',
            'semiMajorAxis' : '{0}au'.format(sma),
            'eccentricity' : eccentricity,
            'period' : '{0}y'.format(period),
            'inclination' : inclination,
            'ascendingNode' : ascendingNode,
            'argumentOfPeriapsis' : argOfPeriapsis,
            'meanAnomaly' : meanAnomaly
            },
          'bodyFrame' : 'EquatorJ2000',
          'geometry' : {
            'type' : 'Mesh',
            'size' : 1.0,
            'source' : model
            }
          }

    #print json.dumps(t, indent = 4) + ','
    print '      {'
    print '         "name" : "{0}",'.format(name)
    print '         "class" : "asteroid",'
    print '         "label" : { "color" : "#aa8855" },'
    print '         "trajectoryPlot": { "fade": 0.3 },'
    print '         "center": "Sun",'
    print '         "trajectoryFrame": "EclipticJ2000",'
    print '         "trajectory": {'
    print '           "type": "Keplerian",'
    print '           "epoch": "{0}",'.format(epochString)
    print '           "period": "{0}y",'.format(period)
    print '           "semiMajorAxis": "{0}au",'.format(sma)
    print '           "eccentricity": {0},'.format(eccentricity)
    print '           "inclination": {0},'.format(inclination)
    print '           "ascendingNode": {0},'.format(ascendingNode)
    print '           "argumentOfPeriapsis": {0},'.format(argOfPeriapsis)
    print '           "meanAnomaly": {0}'.format(meanAnomaly)
    print '         },'
    print '         "bodyFrame": "EquatorJ2000",'
    print '         "geometry": {'
    print '           "source": "{0}",'.format(model)
    print '           "type": "Mesh",'
    print '           "size": {0}'.format(radius)
    print '         }'
    print '      },'
