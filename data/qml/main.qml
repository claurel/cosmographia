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

    property int panelY: 100

    function showFindObject()
    {
        findObject.searchText = "";
        findObject.show()
    }

    Connections
    {
        target: universeView;
        onContextMenuTriggered: {
            contextMenu.show(x, y, body);
        }
    }

    MouseArea {
        id: pageArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked:  {
            timePanel.unfocus();
            mouse.accepted = false
        }
    }

    SettingsPanel {
        id: settingsPanel
        x: 32; y: panelY
        opacity: 0
        textColor: "white"
    }

    TimePanel {
        id: timePanel
        anchors {
            top: parent.top
            right: parent.right
        }
    }

    TextPanel {
        id: helpPanel
        x: 32; y: panelY
        opacity: 0
        textColor: "white"
    }

    Row {
         anchors {
             horizontalCenter: parent.horizontalCenter
         }

         SearchBox {
             id: findObject;
             objectName: "searchBox"
             opacity: 0
         }
     }

     Column {
         id: toolBar

         opacity: 0
         state: ""

         x: 0; y: panelY

         spacing: 10

         Column {
             Image {
                 width: 32; height: 32
                 source: "qrc:/icons/search.png"
                 smooth: true

                 MouseArea {
                     anchors.fill: parent
                     onClicked: { page.showFindObject() }
                 }
             }
             //Text { width: 32; color: "white"; text: "search"; font.pixelSize: 10; horizontalAlignment: Text.AlignHCenter; }
         }

         Column {
             Image {
                 width: 32; height: 32
                 source: "qrc:/icons/config.png"
                 smooth: true

                 MouseArea {
                     anchors.fill: parent
                     onClicked: { settingsPanel.show(); helpPanel.hide() }
                 }
             }
             //Text { width: 32; color: "white"; text: "settings"; font.pixelSize: 10; horizontalAlignment: Text.AlignHCenter; }
         }

         Column {
             Image {
                 width: 32; height: 32
                 source: "qrc:/icons/help.png"
                 smooth: true

                 MouseArea {
                     anchors.fill: parent
                     onClicked: { helpPanel.text = universeView.getHelpText(); helpPanel.show(); settingsPanel.hide() }
                 }
             }
             //Text { width: 32; color: "white"; text: "help"; font.pixelSize: 10; horizontalAlignment: Text.AlignHCenter; }
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

     ContextMenu {
         id: contextMenu
         opacity: 0
     }
 }
