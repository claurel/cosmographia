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
import "LinkStack.js" as BrowserStack

Item {
    id: container

    property string fontFamily: "Century Gothic"
    property int fontSize: 14
    property color textColor: "#72c0ff"
    property alias text: contents.text

    function show()
    {
        state = "visible"
    }

    function hide()
    {
        state = ""
    }

    function navigateTo(link)
    {
        console.log(link);
        if (link.substr(0, 5) == "help:")
        {
            var newText = helpCatalog.getHelpText(link.substr(5));
            if (newText.length > 0)
            {
                BrowserStack.navigateTo(link);
                updateNavigationButtons();
                text = newText;
            }
        }
    }

    function updateNavigationButtons()
    {
        previousButton.opacity = BrowserStack.atBeginning() ? 0.3 : 1.0
        nextButton.opacity = BrowserStack.atEnd() ? 0.3 : 1.0
    }

    function updateContents(helpUrl)
    {
        contents.text = helpCatalog.getHelpText(helpUrl.substr(5));
    }

    Component.onCompleted:
    {
        BrowserStack.setHome("help:help")
    }

    width: 500
    height: 600
    opacity: 0

    Rectangle {
        anchors.fill: parent
        color: "#404040"
        opacity: 0.5
    }

    // Back button
    Image {
        id: previousButton
        width: 20; height: 20
        x: 8
        anchors.top: parent.top
        anchors.topMargin: 4
        source: "qrc:/icons/previous.png"
        smooth: true
        opacity: 0.3

        MouseArea {
            anchors.fill: parent
            onClicked: {
                var u = BrowserStack.navigateBackward();
                if (u.length > 0)
                    updateContents(u);
                updateNavigationButtons();
            }
        }
    }

    // Forward button
    Image {
        id: nextButton
        width: 20; height: 20
        x: 36
        anchors.top: parent.top
        anchors.topMargin: 4
        source: "qrc:/icons/next.png"
        smooth: true
        opacity: 0.3

        MouseArea {
            anchors.fill: parent
            onClicked: {
                var u = BrowserStack.navigateForward();
                if (u.length > 0)
                    updateContents(u);
                updateNavigationButtons();
            }
        }
    }

    // More contents above indicator arrow
    Image {
        width: 16; height: 8
        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.horizontalCenter: parent.horizontalCenter
        source: "qrc:/icons/up.png"
        smooth: true
        opacity: flickable.atYBeginning ? 0 : 1
    }

    // More contents below indicator arrow
    Image {
        width: 16; height: 8
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 4
        anchors.horizontalCenter: parent.horizontalCenter
        source: "qrc:/icons/down.png"
        smooth: true
        opacity: flickable.atYEnd ? 0 : 1
    }

    Flickable
    {
        id: flickable

        clip: true
        anchors.fill: parent
        anchors.topMargin: 40
        anchors.bottomMargin: 20
        anchors.leftMargin: 10
        anchors.rightMargin: 10

        contentWidth: contents.width; contentHeight: contents.height
        flickableDirection: Flickable.VerticalFlick

        Text {
             id: contents
             width: container.width - 40
             //anchors.margins: 12
             color: textColor
             font.family: fontFamily
             font.pixelSize: fontSize
             wrapMode: Text.WordWrap

             onLinkActivated: {
                 if (link.substr(0, 6) == "cosmo:")
                 {
                     universeView.setStateFromUrl(link)
                 }
                 else if (link.substr(0, 5) == "help:")
                 {
                     container.navigateTo(link);
                 }
                 else
                 {
                    Qt.openUrlExternally(link)
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
