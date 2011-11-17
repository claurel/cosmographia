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

Item {
    id: container
    default property alias content: contents.children
    property alias contentWidth: flickable.contentWidth;
    property alias contentHeight: flickable.contentHeight;

    // More contents above indicator arrow
    Image {
        width: 16; height: 8
        anchors.top: parent.top
        anchors.topMargin: 4
        anchors.horizontalCenter: parent.horizontalCenter
        source: "qrc:/icons/up.png"
        smooth: true
        opacity: flickable.atYBeginning ? 0.1 : 1

        MouseArea {
            anchors.fill: parent
            onPressed: { flickable.contentY = Math.max(0, flickable.contentY - 100); }
        }
    }

    // More contents below indicator arrow
    Image {
        width: 16; height: 8
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 4
        anchors.horizontalCenter: parent.horizontalCenter
        source: "qrc:/icons/down.png"
        smooth: true
        opacity: flickable.atYEnd ? 0.1 : 1

        MouseArea {
            anchors.fill: parent
            onPressed: { flickable.contentY = Math.min(flickable.contentHeight, flickable.contentY + 100); }
        }
    }

    Flickable
    {
        id: flickable

        clip: true
        anchors.fill: parent
        anchors.topMargin: 20
        anchors.bottomMargin: 20
        anchors.leftMargin: 10
        anchors.rightMargin: 10

        Item {
            id: contents;
        }

        flickableDirection: Flickable.VerticalFlick
    }
}
