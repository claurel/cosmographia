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

    function show(x, y, body)
    {
        container.x = x
        container.y = y        
        opacity = 1
        menuView.currentIndex = -1

        selection = body
        selectionName = selection.name

        menuModel.clear();
        menuModel.append({ action: "center",      labelText: "Set As Center" });
        menuModel.append({ action: "goto",        labelText: "Go To" });
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
        opacity = 0
    }

    function menuAction(item)
    {
        if (item.action == "bodyaxes")
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
                selection.setVisualizer("sun direction", universeView.createBodyDirectionVisualizer(selection, universeView.getSun()));
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
                selection.setVisualizer("earth direction", universeView.createBodyDirectionVisualizer(selection, universeView.getEarth()));
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        opacity: 0.7
        color: "#404040"
        border.width: 1
        border.color: "#cccccc"
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

        delegate: Row {
            spacing: 3

            Image {
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
}

