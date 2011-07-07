// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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

Item
{
    id: slider
    width: 400; height: 20
    property real minValue: 0
    property real maxValue: 10
    property int xMin: 0
    property int xMax: width - handle.width

    property real value: 0

    onValueChanged: {
        updateHandle()
    }

    function updateHandle()
    {
        // Should be able to use bar.width instead of (slider.width - handle.width), but it
        // isn't being calculated in time to correctly process the first update
        //handle.x = ((value - minValue) * bar.width) / (maxValue - minValue)
        handle.x = (value - minValue) / (maxValue - minValue) * (slider.width - handle.width)
    }

    Rectangle {
        id: bar
        x: handle.width / 2
        height: 1
        width: slider.width - handle.width
        anchors.verticalCenter: parent.verticalCenter
    }

    Rectangle {
        id: firstTick
        x: bar.x
        height: slider.height / 2
        width: 1
        anchors.verticalCenter: parent.verticalCenter
    }

    Rectangle {
        id: lastTick
        x: bar.x + bar.width - 1
        height: slider.height / 2
        width: 1
        anchors.verticalCenter: parent.verticalCenter
    }

    Image {
        id: handle
        width: 12; height: 12
        anchors.bottom: bar.top
        smooth: true
        source: "qrc:/icons/down.png"

        MouseArea {
            id: handleArea
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: slider.xMin;
            drag.maximumX: slider.xMax

            onPositionChanged: {
                slider.value = slider.minValue + (slider.maxValue - slider.minValue) * handle.x / bar.width
            }
        }
    }
}
