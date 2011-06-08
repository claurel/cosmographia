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

Text {
    width: 60

    property bool enabled: false
    property string fontFamily: "Century Gothic"
    property int fontSize: 14
    property color textColor: "#72c0ff"

    signal toggled(bool enabled)

    font.family: fontFamily
    font.pixelSize: fontSize
    font.weight: Font.Bold
    color: textColor
    text: enabled ? "On" : "Off"
    horizontalAlignment: Text.AlignRight

    MouseArea {
        anchors.fill: parent
        onClicked: { parent.enabled = !parent.enabled; toggled(parent.enabled) }
    }
}

