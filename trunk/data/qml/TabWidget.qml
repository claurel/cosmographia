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

    // Forces child items to be added to stack.children
    default property alias content: stack.children

    property int current: 0

    onCurrentChanged: setTabVisibility()
    Component.onCompleted: setTabVisibility()

    function setTabVisibility()
    {
        for (var i = 0; i < stack.children.length; ++i)
        {
            if (i == current)
                stack.children[i].opacity = 1
            else
                stack.children[i].opacity = 0
        }
    }

    Row {
        id: header

        Repeater {
            model: stack.children.length
            delegate: Item {
                width: container.width / stack.children.length;
                height: 32

                // The four rectangles below form the outline of the
                // tab. The top, left, and right lines are only shown when the
                // tab is active, and the bottom line is only shown when the
                // inactive.
                Rectangle {
                    width: parent.width; height: 1
                    anchors { bottom: parent.bottom; bottomMargin: 1 }
                    color: "gray"
                    opacity: container.current == index ? 0 : 1
                }

                Rectangle {
                    width: parent.width; height: 1
                    anchors { top: parent.top; topMargin: 1 }
                    color: "gray"
                    opacity: container.current == index ? 1 : 0
                }

                Rectangle {
                    y: 1
                    width: 1; height: parent.height - 2
                    anchors { left: parent.left; leftMargin: 0 }
                    color: "gray"
                    opacity: container.current == index ? 1 : 0
                }

                Rectangle {
                    y: 1
                    width: 1; height: parent.height - 2
                    anchors { right: parent.right; rightMargin: 0 }
                    color: "gray"
                    opacity: container.current == index ? 1 : 0
                }

                InfoText {
                    horizontalAlignment: Qt.AlignLeft;
                    verticalAlignment: Qt.AlignVCenter
                    anchors.fill: parent
                    text: stack.children[index].title
                    color: container.current == index ? "white" : "#72c0ff"
                    font.bold: container.current == index
                    font.pixelSize: 14
                    elide: Text.ElideRight
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: container.current = index
                }
            }
        }
    }

    Item {
        id: stack
        width: container.width
        anchors.top: header.bottom;
        anchors.bottom: container.bottom
    }
}
