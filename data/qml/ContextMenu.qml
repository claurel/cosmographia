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
    width: 200; height: 300

    property string selectionName: ""
    property variant selection: null

    signal showInfo()

    function show(x, y, body)
    {
        state = "visible"
        wholeWindow.enabled = true;

        container.x = x
        container.y = y        
        menuView.currentIndex = -1

        selection = body
        selectionName = selection.name

        menuModel.clear();
        menuModel.append({ action: "center",      labelText: "Set As Center" });
        menuModel.append({ action: "goto",        labelText: "Go To" });
        menuModel.append({ action: "info",        labelText: "Show Information" });
        menuModel.append({ action: "plot",        labelText: "Plot Trajectory", checked: universeView.hasTrajectoryPlots(body) });
        menuModel.append({ action: "bodyaxes",    labelText: "Body Axes", checked: body.bodyAxes });
        menuModel.append({ action: "frameaxes",   labelText: "Frame Axes", checked: body.frameAxes });
        menuModel.append({ action: "velocity",    labelText: "Velocity Direction", checked: body.velocityArrow });
        if (selectionName != "Sun") {
            menuModel.append({ action: "sun",         labelText: "Sun Direction", checked: body.hasVisualizer("sun direction") });
        }
        if (selectionName != "Earth") {
            menuModel.append({ action: "earth",       labelText: "Earth Direction", checked: body.hasVisualizer("earth direction") });
        }
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
        else if (item.action == "goto")
        {
            universeView.setSelectedBody(selection);
            universeView.gotoSelectedObject();
        }
        else if (item.action == "info")
        {
            universeView.setSelectedBody(selection);
            showInfo()
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

    Rectangle {
        anchors.fill: parent
        opacity: 0.7
        color: "#303030"
        border.width: 1
        border.color: "#606060"
    }

    ListModel {
        id: menuModel
        ListElement { action: "example"; labelText: "Example"; checked: false }
    }

    ListView {
        id: menuView
        model: menuModel

        anchors.fill:  parent
        anchors.margins: 5
        spacing: 3

        interactive: false
        highlightMoveSpeed: 5000

        delegate: Row {
            spacing: 3

            Image {
                anchors.verticalCenter:  parent.verticalCenter
                opacity: checked ? 1.0 : 0.01
                smooth: true
                width: 12; height: 12
                source:  "qrc:/icons/check.png"
            }

            InfoText {
                text: labelText
            }
        }

        header: Column {
            Row {
                Item { width: 16; height: 16 }
                InfoText { text: "<b>" + selectionName + "</b>"; color: "white" }
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

