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
    property color textColor: "#72c0ff"

    function show()
    {
        state = "visible"
    }

    function hide()
    {
        state = ""
    }

    width: 300
    height: 600
    opacity: 0

    Connections
    {
        target: universeView;
        onLimitingMagnitudeChanged: {
            magnitudeSlider.value = universeView.limitingMagnitude
        }

        onAmbientLightChanged: {
            ambientLightSlider.value = universeView.ambientLight
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#404040"
        opacity: 0.5
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

    Item {
        id: title
        width: parent.width
        height: 36

        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 8
            font.family: fontFamily
            font.pixelSize: fontSize
            font.weight: Font.Bold
            color: "white"
            text: "Settings"
        }
    }

    TabWidget {
        anchors.top:  title.bottom
        width: container.width

        Grid {
            property string title: " Graphics"
            y: 30
            anchors {
                left: parent.left
                margins: 8
            }

            columns: 2
            spacing: 8

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Clouds"
            }

            TextToggle {
                onToggled: { universeView.cloudsVisible = enabled }
                enabled: true
            }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Atmospheres"
            }

            TextToggle {
                onToggled: { universeView.atmospheresVisible = enabled }
                enabled: true
            }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Eclipse Shadows"
            }

            TextToggle {
                onToggled: { universeView.eclipseShadows = enabled }
                enabled: false
            }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Other Shadows"
            }

            TextToggle {
                onToggled: { universeView.shadows = enabled }
                enabled: false
            }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Reflections"
            }

            TextToggle {
                onToggled: { universeView.reflections = enabled }
                enabled: false
            }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Sun Glare"
            }

            TextToggle {
                onToggled: { universeView.sunGlare = enabled }
                enabled: true
            }


            // Spacer
            Item { height: 20; width: 10 }
            Item { height: 20; width: 10 }

            Column {
                width: magnitudeSlider.width

                spacing: 5
                Text {
                    font.family: fontFamily
                    font.pixelSize: fontSize
                    color: textColor
                    text: "Star Brightness"
                }

                Slider {
                    id: magnitudeSlider

                    value: 8.0
                    minValue: 3.0
                    maxValue: 13.0

                    width: 200
                    onValueChanged: { universeView.limitingMagnitude = value }
                }
            }
            Item { width: 1; height: 1 }

            // Spacer
            Item { height: 10; width: 10 }
            Item { height: 10; width: 10 }

            Column {
                width: ambientLightSlider.width

                spacing: 5
                Text {
                    font.family: fontFamily
                    font.pixelSize: fontSize
                    color: textColor
                    text: "Extra Light"
                }

                Slider {
                    id: ambientLightSlider

                    value: 0.0
                    minValue: 0.0
                    maxValue: 0.5

                    width: 200
                    onValueChanged: { universeView.ambientLight = value }
                }
            }
            Item {}
        }

        Grid {
            property string title: " Guides"
            y: 30
            anchors {
                left: parent.left
                margins: 8
            }

            columns: 2
            spacing: 8

            // Blank line
            //Item { height: 20; width: 200 } Item { height: 20; width: 20 }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Labels"
            }

            TextToggle {
                enabled: true
                onToggled: { universeView.labelsVisible = enabled }
            }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Center Indicator"
            }

            TextToggle {
                id: centerIndicatorToggle
                enabled: true
                onToggled: { universeView.centerIndicatorVisible = enabled }
            }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Equatorial Grid"
            }

            TextToggle {
                id: equatorialGridToggle
                enabled: false
                onToggled: { universeView.equatorialGridVisible = enabled; }
                Connections
                {
                    target: universeView;
                    onEquatorialGridVisibleChanged: {
                        equatorialGridToggle.enabled = universeView.equatorialGridVisible
                    }
                }
            }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Ecliptic"
            }

            TextToggle {
                onToggled: { universeView.eclipticVisible = enabled }
                enabled: false
            }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Constellation Figures"
            }

            TextToggle {
                onToggled: { universeView.constellationFiguresVisible = enabled }
                enabled: false
            }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "Constellation Names"
            }

            TextToggle {
                onToggled: { universeView.constellationNamesVisible = enabled }
                enabled: false
            }

            Item { height: 20; width: 200 } Item { height: 20; width: 20 }
        }

        Item {
            property string title: " Interface"
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
