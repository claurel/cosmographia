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

Column {
    id: container
    
    InfoText {
        id: timeDisplay
        width: 250
        text: universeView.currentTimeString
    }

    Row {
        id: timeEdit

        property int year: 2000
        property int month: 1
        property int day: 1
        property int hour: 12
        property int minute: 0
        property int second: 0

        property color textColor: state =="" ? "#72c0ff" : "white"

        /*
        function jsdate()
        {
            console.log(new Date(Qt.formatDateTime(currentDate, "yyyy/MM/dd hh:mm:ss")).toUTCString());
            return new Date(Qt.formatDateTime(currentDate, "yyyy/MM/dd hh:mm:ss"));
        }
        */

        function isLeapYear(y)
        {
            return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
        }

        function daysInMonth(y, m)
        {
            if (m == 2)
            {
                return isLeapYear(y) ? 29 : 28;
            }
            else if (m == 4 || m == 6 || m == 9 || m == 11)
            {
                return 30;
            }
            else
            {
                return 31;
            }
        }

        function nextYear()
        {
            year++;
            if (day > daysInMonth(year, month))
                day = daysInMonth(year, month);
        }

        function prevYear()
        {
            year--;
            if (day > daysInMonth(year, month))
                day = daysInMonth(year, month);
        }

        function nextMonth()
        {
            if (month == 12)
            {
                month = 1;
                year = year + 1;
            }
            else
            {
                month++;
            }

            if (day > daysInMonth(year, month))
                day = daysInMonth(year, month);
        }

        function prevMonth()
        {
            if (month == 1)
            {
                month = 12;
                year = year - 1;
            }
            else
            {
                month--;
            }

            if (day > daysInMonth(year, month))
                day = daysInMonth(year, month);
        }

        function nextDay()
        {
            if (day == daysInMonth(year, month))
            {
                day = 1;
                nextMonth();
            }
            else
            {
                day = day + 1;
            }
        }

        function prevDay(d)
        {
            if (day == 1)
            {
                prevMonth();
                day = daysInMonth(year, month);
            }
            else
            {
                day = day - 1;
            }
        }

        InfoText {
            id: yearText
            text: "" + timeEdit.year
            color: timeEdit.textColor

            MouseArea {
                anchors.fill: parent
                onPressed: { parent.focus = true }
            }

            Rectangle {
                anchors.fill: parent
                color: "white"
                opacity: parent.focus ? 0.3 : 0
            }

            Keys.onUpPressed: {
                timeEdit.nextYear()
            }

            Keys.onDownPressed: {
                timeEdit.prevYear()
            }

            Keys.onRightPressed: { monthText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        InfoText {
            text: "-"
            color: timeEdit.textColor
        }

        InfoText {
            id: monthText
            text: Qt.formatDate(new Date(2000, timeEdit.month - 1, 3), "MMM")
            color: timeEdit.textColor

            MouseArea {
                anchors.fill: parent
                onPressed: { parent.focus = true }
            }

            Rectangle {
                anchors.fill: parent
                color: "white"
                opacity: parent.focus ? 0.3 : 0
            }

            Keys.onUpPressed: {
                timeEdit.nextMonth()
            }

            Keys.onDownPressed: {
                timeEdit.prevMonth()
            }

            Keys.onLeftPressed: { yearText.focus = true; }
            Keys.onRightPressed: { dayText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        InfoText {
            text: "-"
            color: timeEdit.textColor
        }

        InfoText {
            id: dayText
            text: timeEdit.day < 10 ? "0" + timeEdit.day : timeEdit.day
            color: timeEdit.textColor

            MouseArea {
                anchors.fill: parent
                onPressed: { parent.focus = true }
            }

            Rectangle {
                anchors.fill: parent
                color: "white"
                opacity: parent.focus ? 0.3 : 0
            }

            Keys.onUpPressed: {
                timeEdit.nextDay()
            }

            Keys.onDownPressed: {
                timeEdit.prevDay()
            }

            Keys.onLeftPressed: { monthText.focus = true; }
            Keys.onRightPressed: { hourText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        InfoText {
            text: " "
        }

        InfoText {
            id: hourText
            text: timeEdit.hour < 10 ? "0" + timeEdit.hour : timeEdit.hour
            color: timeEdit.textColor

            MouseArea {
                anchors.fill: parent
                onPressed: { parent.focus = true }
            }

            Rectangle {
                anchors.fill: parent
                color: "white"
                opacity: parent.focus ? 0.3 : 0
            }

            Keys.onUpPressed: {
                hour = (hour + 1) % 24
            }

            Keys.onDownPressed: {
                hour = (hour - 1) % 24
            }

            Keys.onLeftPressed: { dayText.focus = true; }
            Keys.onRightPressed: { minuteText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        InfoText {
            text: ":"
            color: timeEdit.textColor
        }

        InfoText {
            id: minuteText
            text: timeEdit.minute < 10 ? "0" + timeEdit.minute : timeEdit.minute
            color: timeEdit.textColor

            MouseArea {
                anchors.fill: parent
                onPressed: { parent.focus = true }
            }

            Rectangle {
                anchors.fill: parent
                color: "white"
                opacity: parent.focus ? 0.3 : 0
            }

            Keys.onUpPressed: {
            }

            Keys.onDownPressed: {
            }

            Keys.onLeftPressed: { hourText.focus = true; }
            Keys.onRightPressed: { secondText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        InfoText {
            text: ":"
            color: timeEdit.textColor
        }

        InfoText {
            id: secondText
            text: timeEdit.second < 10 ? "0" + timeEdit.second : timeEdit.second
            color: timeEdit.textColor

            MouseArea {
                anchors.fill: parent
                onPressed: { parent.focus = true }
            }

            Rectangle {
                anchors.fill: parent
                color: "white"
                opacity: parent.focus ? 0.3 : 0
            }

            Keys.onUpPressed: {
            }

            Keys.onDownPressed: {
            }

            Keys.onLeftPressed: { minuteText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        states: State {
            name: "inputActive";
            when: yearText.focus || monthText.focus || dayText.focus || hourText.focus || minuteText.focus || secondText.focus;
        }
    }

    InfoText {
        id: timeScaleDisplay
        text: "Time rate: " + universeView.timeScale           
    }
}

