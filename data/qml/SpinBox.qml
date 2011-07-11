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
    id: container
    width: column.width; height: column.height

    property alias text: textBox.text
    property color color: "white"

    signal up()
    signal down()

    Column
    {
        id: column
        spacing: 2

        Image {
            width: 16; height: 8
            anchors.horizontalCenter: textBox.horizontalCenter

            source: "qrc:/icons/up.png"
            smooth: true
            opacity: container.focus ? 1 : 0.1

            MouseArea {
                anchors.fill: parent
                onPressed: { container.up() }
            }
        }

        InfoText {
            id: textBox
            color: container.color

            MouseArea {
                anchors.fill: parent
                onPressed: { container.focus = true }
            }

            /*
            Rectangle {
                anchors.fill: parent
                color: "white"
                opacity: container.focus ? 0.3 : 0
            }
            */
        }

        Image {
            width: 16; height: 8
            anchors.horizontalCenter: textBox.horizontalCenter

            source: "qrc:/icons/down.png"
            smooth: true
            opacity: container.focus ? 1 : 0.1

            MouseArea {
                anchors.fill: parent
                onPressed: { container.down() }
            }
        }
    }

    Keys.onUpPressed: {
        container.up()
    }

    Keys.onDownPressed: {
        container.down()
    }
}


