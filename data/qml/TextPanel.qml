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

    property string fontFamily: "Century Gothic"
    property int fontSize: 14
    property color textColor: "#72c0ff"
    property alias text: contents.text

    function show()
    {
        state = "visible"
    }

    function hide()
    {
        state = ""
    }

    width: 500
    height: 600
    opacity: 0

    Rectangle {
        anchors.fill: parent
        color: "#404040"
        opacity: 0.5
    }

    Flickable
    {
        clip: true
        anchors.fill: parent
        anchors.margins: 12

        contentWidth: contents.width; contentHeight: contents.height
        flickableDirection: Flickable.VerticalFlick

        Text {
             id: contents
             width: container.width - 40
             //anchors.margins: 12
             color: textColor
             font.family: fontFamily
             font.pixelSize: fontSize
             wrapMode: Text.WordWrap

             onLinkActivated: {
                 if (link.substr(0, 6) == "cosmo:")
                 {
                     universeView.setStateFromUrl(link)
                 }
                 else
                 {
                    Qt.openUrlExternally(link)
                 }
             }
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
