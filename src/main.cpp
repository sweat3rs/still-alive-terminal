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

#include <array>
#include <string>
#include <chrono>

#define LYRIC_WINDOW_SIZE (0.95)
#define CREDITS_WINDOW_SIZE (1.0)

// TODO: Add config file for audio offset

using namespace term;

int main(int argc, char *argv[])
{
    if (argc > 1) {
        if (strcmp(argv[1], "--ignore-warnings") == 0) {
            Window::s_ignoreWarnings = true;
            //std::cout << "Ignoring compatability warnings\n" << std::endl;
        }
        else {
            std::cerr << "Unknown flag '" << argv[1] << "'" << std::endl;
            return 1;
        }
    }

    std::array<Window, 4> windows;

    windows[0] = Window{};
    if (Window::GetGlobalErrorState() != NONE) return 1;

    Vec2 termSize = Window::GetTerminalSize();

    uint32_t windowWidths = (termSize.x/2) - 1;
    uint32_t lyricWindowHeight = termSize.y*LYRIC_WINDOW_SIZE;
    uint32_t creditWindowHeight = termSize.y*LYRIC_WINDOW_SIZE*CREDITS_WINDOW_SIZE*0.5;
    
    windows[1] = Window{windowWidths, lyricWindowHeight, 0 , 0};
    windows[2] = Window{windowWidths, creditWindowHeight, (windowWidths + 2), 0};
    windows[3] = Window{windowWidths, (termSize.y - creditWindowHeight), (windowWidths + 2), creditWindowHeight};

    Window::ChangeColorValue(COLOR_BLACK, Color{0,0,0});
    Window::ChangeColorValue(COLOR_YELLOW, Color{1000, 700, 0});

    for (auto& window : windows) {
        window.FillBackgroundColor(COLOR_BLACK);
        window.SetColor(COLOR_YELLOW, COLOR_BLACK);
        window.Update();
    }
    Window::Refresh();

    windows[1].DrawBorder('|', '|', '-', '-', ' ', ' ', ' ', ' ');
    windows[2].DrawBorder('|', '|', '-', '-', ' ', ' ', ' ', ' ');
    //windows[3].DrawBorder('|', '|', '-', '-', ' ', ' ', ' ', ' ');

    do {
        for (auto& window : windows) {
            window.Update();
        }

        Window::Refresh();
    } while (getch() != 'q');
}

//  ___ ___  ___
// | __/ _ \| __|
// | _| (_) | _|
// |___\___/|_|
//