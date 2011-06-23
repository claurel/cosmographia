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

    property string fontFamily: "Century Gothic"
    property int fontSize: 14
    property color textColor: "white"
    //property alias text: contents.text
    property alias searchText: textInput.text

    function show()
    {
        state = "visible"
        textInput.focus = true
    }

    function hide()
    {
        state = ""
        textInput.focus = false
    }

    width: 400
    height: 300
    opacity: 0

    Rectangle {
        anchors.fill: parent
        color: "#404040"
        opacity: 0.5
    }

    Column {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 5

        Item {
            width: parent.width
            height: 36

            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 0
                font.family: fontFamily
                font.pixelSize: fontSize
                font.weight: Font.Bold
                color: "white"
                text: "Find Object"
            }
        }

        Text {
            color: textColor
            font.family: fontFamily
            font.pixelSize: fontSize
            text: "Enter object name:"
        }

        TextInput {
            id: textInput
            width: 200

            focus: true
            selectByMouse: true
            color: textColor
            font.family: fontFamily
            font.pixelSize: fontSize
            onAccepted: {
                universeView.setSelectedBody(text);
                container.hide();
            }

            onTextChanged: {
                var body = universeView.lookupBody(text);
                if (body)
                {
                    infoText.text = body.name;
                    buttons.opacity = 1;
                }
                else
                {
                    infoText.text = "";
                    buttons.opacity = 0;
                }
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: -2
                color: "transparent"
                border.color: "gray"
                border.width: 1
            }
        }

        Item { width: 1; height: 20 }

        Text {
            id: infoText
            color: textColor
            font.family: fontFamily
            font.pixelSize: fontSize
            text: " "
        }

        Item { width: 1; height: 20 }

        Column {
            id: buttons
            opacity: 0

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                font.weight: Font.Bold
                color: "white"
                text: "Actions:"
            }

            Row {
                spacing: 40

                TextButton {
                    id: selectButton
                    text: "Select"
                    onPressed: { universeView.setSelectedBody(universeView.lookupBody(text)); }
                }

                TextButton {
                    id: centerButton
                    text: "Center"
                }

                TextButton {
                    id: gotoButton
                    text: "Go To"
                }
            }
        }
    }

    Image {
        id: close
        width: 20; height: 20
        smooth: true
        anchors {
            right: parent.right
            rightMargin: 8
            top: parent.top
            topMargin: 8
        }
        source: "qrc:/icons/clear.png"

        MouseArea {
            anchors.fill: parent
            onClicked: { container.hide() }
        }
    }

    states: State {
        name: "visible"
        PropertyChanges { target: container; opacity: 1 }
    }

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
