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
        setActivePanel("findObjectPanel");
    }

    function setActivePanel(name)
    {
        if (name == "findObjectPanel")
        {
            findObjectPanel.searchText = "";
            findObjectPanel.show();
        }
        else
            findObjectPanel.hide();

        if (name == "infoPanel")
            infoPanel.show();
        else
            infoPanel.hide();

        if (name == "settingsPanel")
            settingsPanel.show();
        else
            settingsPanel.hide();

        if (name == "helpPanel")
            helpPanel.show();
        else
            helpPanel.hide();
    }

    Connections
    {
        target: universeView;
        onContextMenuTriggered: {
            var cx = x
            var cy = y

            // Constrain context menu to fit in the main window
            if (cx + contextMenu.width > page.width)
                cx = x - contextMenu.width;
            if (cy + contextMenu.height > page.height)
                cy = page.height - contextMenu.height;

            contextMenu.show(cx, cy, body);
        }
    }

    Connections
    {
        target: contextMenu;
        onShowInfo: {
            var body = universeView.getSelectedBody()
            if (body !== null)
            {
                infoPanel.text = helpCatalog.getHelpText(body.name);
            }
            setActivePanel("infoPanel")
        }
    }

    MouseArea {
        id: pageArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked:  {
            timePanel.unfocus();
            universeView.setMouseClickEventProcessed(false)
        }

        onPositionChanged: {
            universeView.setMouseMoveEventProcessed(false)
        }
    }

    TimePanel {
        id: timePanel
        anchors {
            top: parent.top
            right: parent.right
        }
    }

    SettingsPanel {
        id: settingsPanel
        x: 32; y: panelY
        opacity: 0
        textColor: "white"
    }

    TextPanel {
        id: helpPanel
        width: 400
        x: 32; y: panelY
        opacity: 0
        textColor: "white"
    }

    TextPanel {
        id: infoPanel
        width: 400
        x: 32; y: panelY
        opacity: 0
        textColor: "white"
    }

    FindObjectPanel {
        id: findObjectPanel
        objectName: "searchBox"

        width: 350; height: 300
        x: 32; y: panelY
        opacity: 0
    }

    Column {
         id: toolBar

         opacity: 0
         state: ""

         x: 0; y: panelY

         spacing: 10

         Column {
             Image {
                 id: searchButton
                 width: 32; height: 32
                 source: "qrc:/icons/search.png"
                 smooth: true

                 MouseArea {
                     anchors.fill: parent
                     onClicked: { setActivePanel("findObjectPanel") }
                 }
             }
             //Text { width: 48; smooth: true; color: "white"; text: "search"; font.pixelSize: 10; horizontalAlignment: Text.AlignHCenter; }
         }

         /*
         Column {
             Image {
                 id: infoButton
                 width: 32; height: 32
                 source: "qrc:/icons/info.png"
                 smooth: true

                 MouseArea {
                     anchors.fill: parent
                     onClicked: {
                         var body = universeView.getSelectedBody()
                         if (body !== null)
                         {
                             infoPanel.text = helpCatalog.getHelpText(body.name);
                         }
                         setActivePanel("infoPanel")
                     }
                 }
             }
         }
         */

         Column {
             Image {
                 id: settingsButton
                 width: 32; height: 32
                 source: "qrc:/icons/config.png"
                 smooth: true

                 MouseArea {
                     anchors.fill: parent
                     onClicked: { setActivePanel("settingsPanel") }
                 }
             }
             //Text { width: 48; color: "white"; text: "settings"; font.pixelSize: 10; horizontalAlignment: Text.AlignHCenter; }
         }

         Column {
             Image {
                 id: helpButton
                 width: 32; height: 32
                 source: "qrc:/icons/help.png"
                 smooth: true

                 MouseArea {
                     anchors.fill: parent
                     onClicked: { helpPanel.text = helpCatalog.getHelpText("help"); setActivePanel("helpPanel") }
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

     Image {
         id: quitButton
         width: 32; height: 32
         source: "qrc:/icons/power.png"
         smooth: true
         opacity: toolBar.opacity

         anchors.left: page.left
         anchors.bottom: page.bottom

         MouseArea {
             anchors.fill: parent
             onClicked: { Qt.quit() }
         }
     }

     ContextMenu {
         id: contextMenu
         opacity: 0
     }

     InfoText {
         id: recordingAlert
         anchors.horizontalCenter: page.horizontalCenter
         anchors.bottom: page.bottom
         anchors.bottomMargin: 30
         opacity: universeView.recordingVideo ? 1 : 0

         text: "Recording video (" + universeView.recordedVideoLength.toFixed(1) + " sec)"

         color: "white"
     }
 }
