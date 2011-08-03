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

FocusScope
{
    id: scope
    width: container.width; height: container.height

    property alias text: textBox.text
    property color color: "white"
    property int currentIndex: 0
    property int maxIndex: 0
    property int choiceTextWidth: 60
    property bool wrap: false
    
    function up()
    {    
        if (wrap)
        {
            currentIndex = (currentIndex + 1) % (maxIndex + 1);
        }
        else
        {
            currentIndex = Math.min(maxIndex, currentIndex + 1)
        }
    }

    function down()
    {
        if (wrap)
        {
            if (currentIndex == 0) 
            {
                currentIndex = maxIndex;
            }
            else
            {
                currentIndex--;
            }
        }
        else
        {
            currentIndex = Math.max(0, currentIndex - 1)
        }
    }

    Row
    {
        id: container
        spacing: 10

        PanelText {
            id: textBox
            width: scope.choiceTextWidth
            color: scope.color

            MouseArea {
                anchors.fill: parent
                //onPressed: { scope.focus = true }
            }
        }

        Column
        {
            spacing: 3

            Image {
                id: upArrow
                width: 12; height: 8

                source: "qrc:/icons/up.png"
                smooth: true

                MouseArea {
                    anchors.fill: parent
                    onPressed: { scope.up() }
                }
            }

            Image {
                id:  downArrow
                width: 12; height: 8

                source: "qrc:/icons/down.png"
                smooth: true

                MouseArea {
                    anchors.fill: parent
                    onPressed: { scope.down() }
                }
            }
        }
    }

    Keys.onUpPressed: {
        scope.up()
    }

    Keys.onDownPressed: {
        scope.down()
    }
}


