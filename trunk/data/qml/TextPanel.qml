import QtQuick 1.0

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

    width: 500
    height: 600
    opacity: 0

    anchors {
        horizontalCenter: parent.horizontalCenter
        verticalCenter: parent.verticalCenter
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

    Text {
         id: contents
         anchors.fill: parent
         anchors.margins: 12
         color: textColor
         font.family: fontFamily
         font.pixelSize: fontSize
         wrapMode: Text.WordWrap

         onLinkActivated: { Qt.openUrlExternally(link) }
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
