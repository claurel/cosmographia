// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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

import QtQuick 1.0

Item {
    id: container

    property string fontFamily: "Century Gothic"
    property int fontSize: 14
    property color textColor: "#72c0ff"

    property variant target: false
    property variant center: false

    width: 400
    height: 90
    opacity: 0

    Connections {
        target: universeView;
        onSimulationDateTimeChanged: {
            if (opacity > 0)
            {
                if (target && center)
                {                
                    var distance = target.distanceTo(center, universeView.simulationTime);
                    var speed = target.relativeSpeed(center, universeView.simulationTime);
                    distanceLabel.text = "Distance: " + cosmoApp.formatNumber(distance, 6) + " km";
                    speedLabel.text = "Relative speed: " + cosmoApp.formatNumber(speed, 3) + " km/s";
                }
            }
        }
    }

    function show(_target, _center)
    {
        state = "visible"
        target = _target
        center = _center
        titleLabel.text = _target.name + " \u2192 " + _center.name
    }

    function hide()
    {
        state = "";
    }

    PanelRectangle {
        anchors.fill: parent
    }

    Item {
        id: title
        width: parent.width
        height: 36

        Text {
            id: titleLabel
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 8
            font.family: fontFamily
            font.pixelSize: fontSize
            font.weight: Font.Bold
            color: "white"
            text: "Distance"
        }
    }

    Image {
        id: close
        width: 20; height: 20
        smooth: true
        anchors {
            right: parent.right
            rightMargin: 8
            top: parent.top
            topMargin: 8
        }
        source: "qrc:/icons/clear.png"

        MouseArea {
            anchors.fill: parent
            onClicked: { container.hide() }
        }
    }

    Column {
        anchors.top: title.bottom
        anchors.topMargin: 3
        x: 12

        spacing: 4

        PanelText {
            id: distanceLabel
            color: textColor
        }
        PanelText {
            id: speedLabel
            color: textColor
        }
    }

    states: State {
        name: "visible"
        PropertyChanges { target: container; opacity: 1 }
    }

    transitions: [
        Transition {
            from: ""; to: "visible"
            NumberAnimation { properties: "opacity" }
        },
        Transition {
            from: "visible"; to: ""
            NumberAnimation { properties: "opacity" }
        }
    ]
}

