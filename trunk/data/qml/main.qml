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
    id: page
    width: 1680; height: 1050

    function showFindObject()
    {
        findObject.searchText = "";
        findObject.opacity = 1;
        findObject.focus = true;
    }

    MouseArea {
        id: pageArea
        anchors.fill: parent
        hoverEnabled: true
    }

    TextPanel {
        id: helpPanel
        opacity: 0
        textColor: "white"
        text:
"<b>Cosmographia controls</b>
<br><br>
<b>Mouse</b>
<ul>
<li>Left drag orbits center object</li>
<li>Right drag pans the view</li>
<li>Mouse wheel changes distance to center object</li>
<li>Single click selects an object</li>
<li>Double click an object to visit it</li>
</ul>
<b>Keyboard</b>
<ul>
<li>Arrow keys pan the view</li>
</ul>
<b>Gestures</b>
<ul>
<li>On supported trackpads, pinch or expand to change the field of view</li>
</ul>
<br><br>
For more information, visit <a href=\"http://code.google.com/p/cosmographia/wiki/UserInterface\">the Cosmographia website</a>.
"
    }

    Row {
         anchors {
             horizontalCenter: parent.horizontalCenter
         }

         SearchBox {
             id: findObject;
             objectName: "searchBox"
             focus: true
             opacity: 0
         }
     }

     Column {
         id: toolBar

         opacity: 0
         state: ""

         anchors {
             verticalCenter: parent.verticalCenter
         }

         spacing: 10

         Image {
             width: 32; height: 32
             source: "qrc:/icons/search.png"
             smooth: true

             MouseArea {
                 anchors.fill: parent
                 onClicked: { page.showFindObject() }                 
             }
         }

         Image {
             width: 32; height: 32
             source: "qrc:/icons/config.png"
             smooth: true
         }

         Image {
             width: 32; height: 32
             source: "qrc:/icons/help.png"
             smooth: true

             MouseArea {
                 anchors.fill: parent
                 onClicked: { helpPanel.show() }
             }
         }

         states: [
             State {
                 name: "visible"; when: pageArea.mouseX < 32
                 PropertyChanges { target: toolBar; opacity: 1 }
             }
         ]

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
 }
