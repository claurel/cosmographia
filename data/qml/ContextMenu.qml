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
import Cosmographia 1.0

Item
{
    id: container
    width: 200; height: 348

    property string selectionName: ""
    property variant selection: null

    signal showInfo()
    signal showProperties()
    signal showDistance(variant target, variant center)

    function show(x, y, body)
    {
        state = "visible"
        wholeWindow.enabled = true;

        container.x = x
        container.y = y        
        menuView.currentIndex = -1

        selection = body
        selectionName = selection.name

        // We'll guess the height of the menu based on the height
        // of an individual menu item
        var c = Qt.createQmlObject("InfoText { text: 'Test' }", container);
        var rowHeight = c.height + 3;
        c.opacity = 0;  // hide the test item

        menuModel.clear();
        menuModel.append({ action: "goto",        labelText: "Go To", checked: false, type: "camera" });
        menuModel.append({ action: "center",      labelText: "Set As Center", checked: false, type: "camera" });
        menuModel.append({ action: "fixcenter",   labelText: "Set As Fixed Center", checked: false, type: "camera" });
        menuModel.append({ action: "track",       labelText: "Track", checked: false, type: "camera" });

        menuModel.append({ action: "none",        labelText: " ", checked: false, type: "info" });
        menuModel.append({ action: "description",        labelText: "Show Description", checked: false, type: "info" });
        menuModel.append({ action: "properties",        labelText: "Show Properties", checked: false, type: "info" });

        var targetBodyName = universeView.getSelectedBody().name;
        if (selectionName != targetBodyName && targetBodyName != "")
        {
            menuModel.append({ action: "distance", labelText: "Distance to " + targetBodyName, checked: false, type: "info" })
        }

        menuModel.append({ action: "none",        labelText: " ", checked: false, type: "camera" });

        menuModel.append({ action: "plot",        labelText: "Plot Trajectory", checked: universeView.hasTrajectoryPlots(body), type: "camera" });
        if (body.isEllipsoid)
        {       
            menuModel.append({ action: "grid",        labelText: "Long/Lat Grid", checked: body.longLatGrid, type: "camera" });
        }

        menuModel.append({ action: "bodyaxes",    labelText: "Body Axes", checked: body.bodyAxes, type: "vectors" });
        menuModel.append({ action: "frameaxes",   labelText: "Frame Axes", checked: body.frameAxes, type: "vectors" });
        menuModel.append({ action: "velocity",    labelText: "Velocity Direction", checked: body.velocityArrow, type: "vectors" });
        if (selectionName != "Sun") {
            menuModel.append({ action: "sun",         labelText: "Sun Direction", checked: body.hasVisualizer("sun direction"), type: "vectors" });
        }
        if (selectionName != "Earth") {
            menuModel.append({ action: "earth",       labelText: "Earth Direction", checked: body.hasVisualizer("earth direction"), type: "vectors" });
        }

        height = menuModel.count * rowHeight + 55;
    }

    function hide()
    {
        state = ""
        wholeWindow.enabled = false
    }

    function menuAction(item)
    {
        if (!item)
        {
            // nothing to do
        }
        else if (item.action == "center")
        {
            universeView.setCentralBody(selection);
        }
        else if (item.action == "fixcenter")
        {
            universeView.setCentralBodyFixed(selection);
        }
        else if (item.action == "track")
        {
            universeView.trackBody(selection);
        }
        else if (item.action == "goto")
        {
            universeView.setSelectedBody(selection);
            universeView.gotoSelectedObject();
        }
        else if (item.action == "description")
        {
            universeView.setSelectedBody(selection);
            showInfo()
        }
        else if (item.action == "properties")
        {
            universeView.setSelectedBody(selection);
            showProperties()
        }
        else if (item.action == "plot")
        {
            universeView.setSelectedBody(selection);
            if (universeView.hasTrajectoryPlots(selection))
                universeView.clearTrajectoryPlots(selection);
            else
                universeView.plotTrajectory(selection);
        }
        else if (item.action == "bodyaxes")
        {
            selection.bodyAxes = !selection.bodyAxes;
        }
        else if (item.action == "frameaxes")
        {
            selection.frameAxes = !selection.frameAxes;
        }
        else if (item.action == "velocity")
        {
            selection.velocityArrow = !selection.velocityArrow;
        }
        else if (item.action == "grid")
        {
            selection.longLatGrid = !selection.longLatGrid;
        }
        else if (item.action == "sun")
        {
            if (selection.hasVisualizer("sun direction"))
            {
                selection.removeVisualizer("sun direction");
            }
            else
            {
                selection.setVisualizer("sun direction", universeView.createBodyDirectionVisualizer(selection, universeCatalog.getSun()));
            }
        }
        else if (item.action == "earth")
        {
            if (selection.hasVisualizer("earth direction"))
            {
                selection.removeVisualizer("earth direction");
            }
            else
            {
                selection.setVisualizer("earth direction", universeView.createBodyDirectionVisualizer(selection, universeCatalog.getEarth()));
            }
        }
        else if (item.action == "distance")
        {
            showDistance(selection, universeView.getSelectedBody())
        }
    }

    // This mouse area is activated when the context menu is shown. It is used
    // to dismiss the context menu when the user clicks outside of it.
    MouseArea {
        id: wholeWindow
        x: -5000; y: -5000
        width: 10000; height: 10000;
        enabled: false
        onClicked: { parent.hide(); }
    }

    PanelRectangle {
        anchors.fill: parent
    }

    ListModel {
        id: menuModel
        ListElement { action: "example"; labelText: "Example"; checked: false }
    }

    ListView {
        id: menuView
        model: menuModel

        anchors.fill:  parent
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        anchors.topMargin: 10
        anchors.bottomMargin: 5
        spacing: 3

        interactive: false
        highlightMoveSpeed: 5000

        delegate: Item {
            id: menuItem
            width: row.width; height: row.height
            Row {
                id: row
                spacing: 3

                Image {
                    anchors.verticalCenter:  parent.verticalCenter
                    opacity: checked ? 1.0 : 0.01
                    smooth: true
                    width: 12; height: 12
                    source:  "qrc:/icons/check.png"
                }

                InfoText {
                    text: labelText;
                    color: "white";
                    width: 180;
                    elide: Text.ElideMiddle;
                }
            }

            Rectangle {
                x: 4; width: 180; height: 1;
                opacity: labelText === " " ? 0.1 : 0.0
                anchors.verticalCenter:  parent.verticalCenter
            }
        }

        header: Column {
            Row {
                Item { width: 16; height: 16 }
                InfoText {
                    width: 180;
                    text: selectionName
                    font.pixelSize: 24
                    elide: Text.ElideRight
                    color: "white"
                }
            }
            Item { width: 1; height: 5 }
        }

        highlight: Rectangle { color: "gray"; opacity: 0.7; x: -4; width: 200; }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onPositionChanged: { parent.currentIndex = parent.indexAt(mouseX, mouseY); }
            onClicked: {
                container.menuAction(parent.model.get(parent.currentIndex));
                container.hide();
            }
        }
    }

    states: State {
        name: "visible"
        PropertyChanges { target: container; opacity: 1 }
    }

    transitions: [
        Transition {
            from: ""; to: "visible"
            NumberAnimation { properties: "opacity"; duration: 125 }
        },
        Transition {
            from: "visible"; to: ""
            NumberAnimation { properties: "opacity"; duration: 125 }
        }
    ]
}

