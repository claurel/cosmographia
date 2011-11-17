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
import "../addons.js" as AddonCatalog


ScrollablePane
{
    id: container

    width: 300; height: 400;

    contentWidth: addonView.width;
    contentHeight: addonView.height;

    Component.onCompleted: {
        populateList()
    }

    function populateList()
    {
        addonModel.clear();
        for (var i = 0; i < AddonCatalog.InstalledAddons.length; i++)
        {
            addonModel.append(AddonCatalog.InstalledAddons[i]);
        }
    }

    Item {
        ListModel {
            id: addonModel
            ListElement {
                name: "Example"; description: "Example Description"; source: "example.json"
            }
        }
    }

    ListView {
        id: addonView
        model: addonModel
        width: 300
        height: 900

        spacing: 10

        interactive: false
        highlightMoveSpeed: 5000

        delegate: Row {
            id: row
            spacing: 6

            TextToggle {
                width: 40;
                horizontalAlignment: Text.AlignHCenter;
                onToggled: {
                    if (enabled)
                        cosmoApp.loadAddOn(source);
                    else
                        cosmoApp.unloadAddOn(source);
                }
            }

            PanelText {
                text: "<b>" + name + "</b><br>" + description
                width: 230
                wrapMode: Text.WordWrap
            }
        }
    }

}
