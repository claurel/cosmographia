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
    width: 250
    id: container
    
    /*
    InfoText {
        id: timeDisplay
        width: 250
        text: universeView.currentTimeString
    }
    */
    function unfocus()
    {
        timeEdit.unfocus()
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

        Connections {
            target: universeView;
            onSimulationDateTimeChanged: {
                if (timeEdit.state != "inputActive")
                {
                    var d = universeView.simulationDateTime;
                    timeEdit.year = d.getUTCFullYear();
                    timeEdit.month = d.getUTCMonth() + 1;
                    timeEdit.day = d.getUTCDate();
                    timeEdit.hour = d.getUTCHours();
                    timeEdit.minute = d.getUTCMinutes();
                    timeEdit.second = d.getUTCSeconds();
                }
            }
        }

        function unfocus()
        {
            yearText.focus = false
            monthText.focus = false
            dayText.focus = false
            hourText.focus = false
            minuteText.focus = false
            secondText.focus = false
        }

        function currentDate()
        {
            var d = new Date();
            /*
            d.setUTCFullYear(year);
            d.setUTCMonth(month - 1);
            d.setUTCDate(day);
            d.setUTCHours(hour);
            d.setUTCMinutes(minute);
            d.setUTCSeconds(second);
            */
            d.setFullYear(year);
            d.setMonth(month - 1);
            d.setDate(day);
            d.setHours(hour);
            d.setMinutes(minute);
            d.setSeconds(second);
            return d;
        }

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

        function nextHour()
        {
            if (hour == 23)
            {
                hour = 0;
                nextDay();
            }
            else
            {
                hour++;
            }
        }

        function prevHour()
        {
            if (hour == 0)
            {
                hour = 23;
                prevDay();
            }
            else
            {
                hour--;
            }
        }

        function nextMinute()
        {
            if (minute == 59)
            {
                minute = 0;
                nextHour();
            }
            else
            {
                minute++;
            }
        }

        function prevMinute()
        {
            if (minute == 0)
            {
                minute = 59;
                prevHour();
            }
            else
            {
                minute--;
            }
        }

        function nextSecond()
        {
            if (second == 59)
            {
                second = 0;
                nextMinute();
            }
            else
            {
                second++;
            }
        }

        function prevSecond()
        {
            if (second == 0)
            {
                second = 59;
                prevMinute();
            }
            else
            {
                second--;
            }
        }

        SpinBox {
            id: yearText
            text: timeEdit.year + "-"
            color: timeEdit.textColor

            onUp: {
                timeEdit.nextYear()
                universeView.simulationDateTime = timeEdit.currentDate()
            }

            onDown: {
                timeEdit.prevYear()
                universeView.simulationDateTime = timeEdit.currentDate()
            }

            Keys.onRightPressed: { monthText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        SpinBox {
            id: monthText
            text: Qt.formatDate(new Date(2000, timeEdit.month - 1, 3), "MMM") + "-"
            color: timeEdit.textColor

            onUp: {
                timeEdit.nextMonth()
                universeView.simulationDateTime = timeEdit.currentDate()
            }

            onDown: {
                timeEdit.prevMonth()
                universeView.simulationDateTime = timeEdit.currentDate()
            }

            Keys.onLeftPressed: { yearText.focus = true; }
            Keys.onRightPressed: { dayText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        SpinBox {
            id: dayText
            text: (timeEdit.day < 10 ? "0" + timeEdit.day : timeEdit.day) + " "
            color: timeEdit.textColor

            onUp: {
                timeEdit.nextDay()
                universeView.simulationDateTime = timeEdit.currentDate()
            }

            onDown: {
                universeView.simulationDateTime = timeEdit.currentDate()
                timeEdit.prevDay()
            }

            Keys.onLeftPressed: { monthText.focus = true; }
            Keys.onRightPressed: { hourText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        SpinBox {
            id: hourText
            text: (timeEdit.hour < 10 ? "0" + timeEdit.hour : timeEdit.hour) + ":"
            color: timeEdit.textColor

            onUp: {
                timeEdit.nextHour()
                universeView.simulationDateTime = timeEdit.currentDate()
            }

            onDown: {
                timeEdit.prevHour()
                universeView.simulationDateTime = timeEdit.currentDate()
            }

            Keys.onLeftPressed: { dayText.focus = true; }
            Keys.onRightPressed: { minuteText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        SpinBox {
            id: minuteText
            text: (timeEdit.minute < 10 ? "0" + timeEdit.minute : timeEdit.minute) + ":"
            color: timeEdit.textColor

            onUp: {
                timeEdit.nextMinute()
                universeView.simulationDateTime = timeEdit.currentDate()
            }

            onDown: {
                timeEdit.prevMinute()
                universeView.simulationDateTime = timeEdit.currentDate()
            }

            Keys.onLeftPressed: { hourText.focus = true; }
            Keys.onRightPressed: { secondText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        SpinBox {
            id: secondText
            text: timeEdit.second < 10 ? "0" + timeEdit.second : timeEdit.second
            color: timeEdit.textColor

            onUp: {
                timeEdit.nextSecond()
                universeView.simulationDateTime = timeEdit.currentDate()
            }

            onDown: {
                timeEdit.prevSecond()
                universeView.simulationDateTime = timeEdit.currentDate()
            }

            Keys.onLeftPressed: { minuteText.focus = true; }
            Keys.onReturnPressed:  { focus = false; }
        }

        InfoText {
            id: timeMode
            text: " UTC"
            color: timeEdit.textColor
            anchors.verticalCenter: secondText.verticalCenter
        }

        states: State {
            name: "inputActive";
            when: yearText.focus || monthText.focus || dayText.focus || hourText.focus || minuteText.focus || secondText.focus;
        }
    }

    InfoText {
        id: timeScaleDisplay
        text: "Time rate: " + universeView.timeScale + (universeView.paused ? "x (paused)" : "x")
    }
}

