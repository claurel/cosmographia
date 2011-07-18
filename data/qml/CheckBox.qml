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
    width: 80; height: 20

    property alias text: label.text
    property bool checked: false
    signal clicked()

    Row {
        spacing: 5

        Image {
            //anchors.verticalCenter:  parent.verticalCenter
            opacity: container.checked ? 1.0 : 0.01
            smooth: true
            width: 12; height: 12
            source:  "qrc:/icons/check.png"
        }

        PanelText {
            id: label
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: { container.clicked(); }
    }
}

