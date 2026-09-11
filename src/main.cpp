/*
    Copyright (C) 2026  Miradge/Sweat3rs

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Window.h"

using namespace term;

int main()
{
    Window mainWindow{};
    mainWindow.Print("Hello, World!");

    while (getch() != 'q') {
        mainWindow.Update();
    }
}

//  ___ ___  ___
// | __/ _ \| __|
// | _| (_) | _|
// |___\___/|_|
//