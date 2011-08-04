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

            PanelText { text: "Milky Way" }
            TextToggle {
                onToggled: { universeView.milkyWayVisible = enabled }
                enabled: false
            }

            PanelText { text: "Clouds" }
            TextToggle {
                onToggled: { universeView.cloudsVisible = enabled }
                enabled: true
            }

            PanelText { text: "Atmospheres" }
            TextToggle {
                onToggled: { universeView.atmospheresVisible = enabled }
                enabled: true
            }

            PanelText { text: "Eclipse Shadows" }
            TextToggle {
                onToggled: { universeView.eclipseShadows = enabled }
                Component.onCompleted: { enabled = universeView.eclipseShadows }
                enabled: false
            }

            PanelText { text: "Other Shadows" }
            TextToggle {
                onToggled: { universeView.shadows = enabled }
                enabled: false
            }

            PanelText { text: "Reflection" }
            TextToggle {
                onToggled: { universeView.reflections = enabled }
                enabled: false
            }

            PanelText { text: "Sun Glare" }
            TextToggle {
                onToggled: { universeView.sunGlare = enabled }
                enabled: true
            }

            PanelText { text: "Diffraction Spikes" }
            TextToggle {
                onToggled: { universeView.diffractionSpikes = enabled }
                Component.onCompleted: { enabled = universeView.diffractionSpikes }
                enabled: false
            }

            // Spacer
            Item { height: 5; width: 10 }
            Item { height: 5; width: 10 }

            Row {
                spacing: 25
                PanelText { text: "Earth Map:" }
                ChoiceBox {
                    maxIndex: 11
                    choiceTextWidth: 75
                    wrap: true
                    text: Qt.formatDate(new Date(2000, currentIndex, 3), "MMMM")
                    onCurrentIndexChanged: { universeView.earthMapMonth = currentIndex }
                    Component.onCompleted: { currentIndex = universeView.earthMapMonth }
                }
            }
            Item { height: 5; width: 5 }

            // Spacer
            Item { height: 15; width: 10 }
            Item { height: 15; width: 10 }

            Column {
                width: magnitudeSlider.width

                spacing: 5

                PanelText { text: "Star Brightness" }
                Slider {
                    id: magnitudeSlider

                    minValue: 4.0
                    maxValue: 13.0
                    value: 1.0

                    width: 200
                    onValueChanged: { universeView.limitingMagnitude = value }
                    Component.onCompleted: { value = universeView.limitingMagnitude }
                }
            }
            Item { width: 1; height: 1 }

            Item { height: 5; width: 10 }
            Item { height: 5; width: 10 }

            Column {
                width: ambientLightSlider.width

                spacing: 5

                PanelText { text: "Extra Light" }
                Slider {
                    id: ambientLightSlider

                    value: 0.0
                    minValue: 0.0
                    maxValue: 0.5

                    width: 200
                    onValueChanged: { universeView.ambientLight = value }
                    Component.onCompleted: { value = universeView.ambientLight }
                }
            }
            Item { width: 1; height: 1 }

            // Spacer
            Item { height: 5; width: 10 }
            Item { height: 5; width: 10 }

            PanelText {
                text: "Anaglyph Stereo"
            }

            TextToggle {
                onToggled: {
                    if (enabled)
                    {
                        universeView.stereoMode = UniverseView.AnaglyphRedCyan;
                    }
                    else
                    {
                        universeView.stereoMode = UniverseView.Mono;
                    }
                }
                enabled: false
            }

            PanelText {
                text: "Anti-aliasing<br><font size=-1>(requires restarting Cosmographia)</font>";
            }

            TextToggle {
                Component.onCompleted: { enabled = universeView.antialiasingSamples > 1 }
                onToggled: {
                    universeView.antialiasingSamples = enabled ? 4 : 1;
                }
                enabled: false
            }

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

            PanelText { text: "Labels" }
            TextToggle {
                enabled: true
                onToggled: { universeView.labelsVisible = enabled }
            }

            PanelText { text: "Planet Orbits" }
            TextToggle {
                enabled: true
                onToggled: { universeView.planetOrbitsVisible = enabled }
            }

            PanelText { text: "Center Indicator" }
            TextToggle {
                id: centerIndicatorToggle
                enabled: true
                onToggled: { universeView.centerIndicatorVisible = enabled }
            }

            PanelText { text: "Equatorial Grid" }
            TextToggle {
                id: equatorialGridToggle
                onToggled: { universeView.equatorialGridVisible = enabled }
            }

            PanelText { text: "Ecliptic" }
            TextToggle {
                onToggled: { universeView.eclipticVisible = enabled }
                enabled: false
            }

            PanelText { text: "Constellation Figures" }
            TextToggle {
                onToggled: { universeView.constellationFiguresVisible = enabled }
                Component.onCompleted: { enabled = universeView.constellationFiguresVisible }
                enabled: false
            }

            PanelText { text: "Constellation Names" }
            TextToggle {
                onToggled: { universeView.constellationNamesVisible = enabled }
                Component.onCompleted: { enabled = universeView.constellationNamesVisible }
                enabled: false
            }

            PanelText { text: "Star Names" }
            TextToggle {
                onToggled: { universeView.starNamesVisible = enabled }
                Component.onCompleted: { enabled = universeView.starNamesVisible }
                enabled: false
            }

            Item { height: 20; width: 200 } Item { height: 20; width: 20 }
        }

        Grid {
            property string title: " Interface"
            y: 30
            anchors {
                left: parent.left
                margins: 8
            }

            columns: 2
            spacing: 8

            Column {
                width: gotoTimeSlider.width

                spacing: 5

                PanelText { text: "Goto Time" }
                Slider {
                    id: gotoTimeSlider

                    value: 5.0
                    minValue: 1.0
                    maxValue: 20.0

                    width: 200
                    onValueChanged: { universeView.gotoObjectTime = value }
                    Component.onCompleted: { value = universeView.gotoObjectTime }
                }
            }

            Text {
                font.family: fontFamily
                font.pixelSize: fontSize
                color: textColor
                text: "" + Math.round(gotoTimeSlider.value) + " sec";
                verticalAlignment: Text.AlignBottom
            }

            // Spacer
            Item { height: 10; width: 10 }
            Item { height: 10; width: 10 }

            PanelText { text: "Auto-hide Tool Bar" }
            TextToggle {
                onToggled: { cosmoApp.autoHideToolBar = enabled }
                enabled: false
                Component.onCompleted: { enabled = cosmoApp.autoHideToolBar }
            }

            // Spacer
            Item { height: 10; width: 10 }
            Item { height: 10; width: 10 }

            PanelText { text: "Recorded Video Resolution" }
            Item { height: 10; width: 10 }

            CheckBox {
                text: "640 x 480 (VGA)"
                checked: cosmoApp.videoSize == "vga"
                onClicked: { cosmoApp.setVideoSize("vga") }
            }
            Item { height: 1; width: 1 }

            CheckBox {
                text: "854 x 480 (WVGA)"
                checked: cosmoApp.videoSize == "wvga"
                onClicked: { cosmoApp.setVideoSize("wvga") }
            }            
            Item { height: 1; width: 1 }

            CheckBox {
                text: "1280 x 720 (720p)"
                checked: cosmoApp.videoSize == "720p"
                onClicked: { cosmoApp.setVideoSize("720p") }
            }            
            Item { height: 1; width: 1 }
        }

        AddonManager
        {
            property string title: " Add-ons"
            width: 300; height: 500;
            y: 16
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
