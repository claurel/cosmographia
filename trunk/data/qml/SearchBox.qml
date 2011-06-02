import QtQuick 1.0

FocusScope {
     id: container

     property string fontFamily: "Century Gothic"
     property int fontSize: 20
     property color textColor: "#72c0ff"
     property alias searchText: textInput.text

     signal searchEntered(string text)

     width: 250; height: 32

     Rectangle {
         width: parent.width; height: parent.height
         color: "gray"
         opacity: 0.4
     }

     Text {
         id: placeholderText
         anchors.fill: parent; anchors.leftMargin: 8
         verticalAlignment: Text.AlignVCenter
         text: "Enter object name..."
         color: textColor
         opacity: 0.5
         font.italic: true
         font.family: fontFamily
         font.pixelSize: fontSize
     }

     MouseArea {
         anchors.fill: parent
         onClicked: { container.focus = true }
     }

     TextInput {
         id: textInput
         anchors { left: parent.left;
                   leftMargin: 8;
                   right: clear.left;
                   rightMargin: 8;
                   verticalCenter: parent.verticalCenter
         }
         focus: true
         selectByMouse: true
         color: textColor
         font.family: fontFamily
         font.pixelSize: fontSize
         onAccepted: { container.opacity = 0; container.searchEntered(text); clear.opacity = 0 }
     }

     Image {
         id: clear
         width: 20; height: 20
         smooth: true
         anchors { right: parent.right; rightMargin: 8; verticalCenter: parent.verticalCenter }
         source: "qrc:/icons/clear.png"
         opacity: 0

         MouseArea {
             anchors.fill: parent
             onClicked: { textInput.text = ''; container.focus = true }
         }
     }

     states: State {
         name: "hasText"; when: textInput.text != ''
         PropertyChanges { target: placeholderText; opacity: 0 }
         PropertyChanges { target: clear; opacity: 0.5 }
     }

     transitions: [
         Transition {
             from: ""; to: "hasText"
             NumberAnimation { exclude: placeholderText; properties: "opacity" }
         },
         Transition {
             from: "hasText"; to: ""
             NumberAnimation { properties: "opacity" }
         }
     ]
 }
