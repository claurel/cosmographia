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
    width: buttonRow.width; height: buttonRow.height

    Row {
        id: buttonRow
        spacing: 8

        Image {
            id: reverseButton
            width: 21; height: 14
            source: "qrc:/icons/reverse.png"
            smooth: true

            MouseArea {
                anchors.fill: parent
                onClicked: { universeView.timeScale = -universeView.timeScale }
            }
        }

        Image {
            id: slowerButton
            width: 21; height: 14
            source: "qrc:/icons/slower.png"
            smooth: true

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (universeView.paused)
                    {
                        universeView.paused = false;
                    }
                    else if (universeView.timeScale > 0 && universeView.timeScale <= 1)
                    {
                        universeView.timeScale = -1
                    }
                    else if (universeView.timeScale < 0)
                    {
                        universeView.timeScale = Math.max(-1.0e7, universeView.timeScale * 10);
                    }
                    else
                    {
                        universeView.timeScale *= 0.1;
                    }
                }
            }
        }

        Image {
            id: pauseButton
            width: 21; height: 14
            source: universeView.paused ? "qrc:/icons/play.png" : "qrc:/icons/pause.png"
            smooth: true

            MouseArea {
                anchors.fill: parent
                onClicked: { universeView.paused = !universeView.paused }
            }
        }

        /*
        Image {
            id: playButton
            width: 24; height: 16
            source: "qrc:/icons/play.png"
            smooth: true

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    universeView.timeScale = 1
                    if (universeView.paused)
                    {
                        universeView.paused = false;
                    }
                }
            }
        }
        */

        Image {
            id: fasterButton
            width: 21; height: 14
            source: "qrc:/icons/faster.png"
            smooth: true

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (universeView.paused)
                    {
                        universeView.paused = false;
                    }
                    else if (universeView.timeScale < 0 && universeView.timeScale >= -1)
                    {
                        universeView.timeScale = 1;
                    }
                    else if (universeView.timeScale > 0)
                    {
                        universeView.timeScale = Math.min(1.0e7, universeView.timeScale * 10);
                    }
                    else
                    {
                        universeView.timeScale *= 0.1;
                    }
                }
            }
        }
    }
}
