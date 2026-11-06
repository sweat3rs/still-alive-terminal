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
#include "AudioPlayer.h"
#include "LuaReader.h"
#include "LinePrinter.h"

#include <array>

#include <filesystem>

#include <string>
#include <chrono>
#include <mutex>

#define LYRIC_WINDOW_SIZE 0.95
#define CREDITS_WINDOW_SIZE 1.0
#define BUFFER_SIZE 1

#define INFO_MESSAGE "Starting... Press \'q\' to quit."
#define LUA_CONFIG_TABLE "config"

std::string CONFIG_FILE{"config.lua"};

#ifdef SOURCE_RUNTIME_DIRECTORY
std::string RUNTIME_DIR{SOURCE_RUNTIME_DIRECTORY};
#else
std::string RUNTIME_DIR{};
#endif

// TODO: Add SIGWINCH handler

using namespace term;
using namespace std::chrono_literals;

int _argument_handler(const char* argument, int* i_ptr, int argc, char* argv[])
{
    if (strcmp(argument, "--ignore-warnings") == 0) {
        Window::s_ignoreWarnings = true;
        //std::cout << "Ignoring compatability warnings\n" << std::endl;
    }
    else if (strcmp(argument, "--config") == 0) {
        *i_ptr = (*i_ptr) + 1;
        if ((*i_ptr) >= argc) return 0;
    
        CONFIG_FILE = argv[*i_ptr];
    }
    else if (strcmp(argument, "--runtime") == 0) {
        *i_ptr = (*i_ptr) + 1;
        if ((*i_ptr) >= argc) return 0;
    
        RUNTIME_DIR = argv[*i_ptr];
        if (RUNTIME_DIR.back() != '/') {
            RUNTIME_DIR = RUNTIME_DIR + "/";
        }
    }
    else {
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    // Arguments
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            auto result = _argument_handler(argv[i], &i, argc, argv);
            if (result == -1) {
                std::cerr << "Unknown flag '" << argv[1] << "'" << std::endl;
                return 1;
            }
        }
    }

    std::cout << INFO_MESSAGE << std::endl;

    // Config File
    if (!std::filesystem::exists(RUNTIME_DIR)) {
        std::cerr << "Runtime directory does not exist!" << std::endl;
        return 1;
    }

    Lua lua{};
    {
        std::string filepath = std::string{RUNTIME_DIR} + CONFIG_FILE;
        lua.AccessTableFromFile(filepath, LUA_CONFIG_TABLE);
    }

    if (lua.GetError()) return 1;

    auto volume = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "volume");
    auto song_file = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "song_file");
    auto start_delay = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "start_delay");
    auto print_speed = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "print_speed");
    auto icons_file = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "icons_file");
    auto lyrics_file = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "lyrics_file");

    if (volume.type != Lua::NUMBER || song_file.type != Lua::STRING || start_delay.type != Lua::NUMBER
        || print_speed.type != Lua::NUMBER || icons_file.type != Lua::STRING || lyrics_file.type != Lua::STRING) {
        std::cerr << "Lua config file parsing failed!" << std::endl;
        return 1;
    }

    // Windows
    std::array<Window, 7> windows;
    // 0 - main
    // 1, 2, 3 - borders
    // 4, 5, 6 - 3 windows

    windows[0] = Window{};
    if (Window::GetGlobalErrorState() != NONE) return 1;

    Vec2 termSize = Window::GetTerminalSize();

    uint32_t windowWidths = (termSize.x/2) - 1 - BUFFER_SIZE;
    uint32_t lyricWindowHeight = termSize.y*LYRIC_WINDOW_SIZE - BUFFER_SIZE;
    uint32_t creditWindowHeight = termSize.y*LYRIC_WINDOW_SIZE*CREDITS_WINDOW_SIZE*0.5 - BUFFER_SIZE;
    
    windows[1] = Window{windowWidths, lyricWindowHeight, BUFFER_SIZE, BUFFER_SIZE};
    windows[2] = Window{windowWidths, creditWindowHeight, (windowWidths + 2) + BUFFER_SIZE, BUFFER_SIZE};
    windows[3] = Window{windowWidths, (termSize.y - creditWindowHeight) - BUFFER_SIZE*2, (windowWidths + 2) + BUFFER_SIZE, creditWindowHeight + BUFFER_SIZE};

    windows[4] = Window{windowWidths - 2, lyricWindowHeight - 2, BUFFER_SIZE + 1, BUFFER_SIZE + 1};
    windows[5] = Window{windowWidths - 2, creditWindowHeight - 2, (windowWidths + 2) + BUFFER_SIZE + 1, BUFFER_SIZE + 1};
    windows[6] = Window{windowWidths - 2, (termSize.y - creditWindowHeight) - BUFFER_SIZE*2 - 2, (windowWidths + 2) + BUFFER_SIZE + 1, creditWindowHeight + BUFFER_SIZE + 1};

    Window::ChangeColorValue(COLOR_BLACK, Color{30,30,30});
    Window::ChangeColorValue(COLOR_YELLOW, Color{1000, 700, 0});

    for (auto& window : windows) {
        window.FillBackgroundColor(COLOR_BLACK);
        window.SetColor(COLOR_YELLOW, COLOR_BLACK);
    }

    Window::Update();

    windows[1].DrawBorder('|', '|', '-', '-', ' ', ' ', ' ', ' ');
    windows[2].DrawBorder('|', '|', '-', '-', ' ', ' ', ' ', ' ');
    //windows[3].DrawBorder('|', '|', '-', '-', ' ', ' ', ' ', ' ');

    windows[4].SetCursorHomePos({1, 0});

    std::array<std::mutex, 3> windowMutexes{};

    // Text animations
    CursorFlash cursorFlash{windows[4], windowMutexes[1], '_', 600ms};
    LinePrinter lyricPrinter(windows[4], windowMutexes[1], print_speed.value.number);
    {
        std::string filepath = std::string(RUNTIME_DIR) + lyrics_file.value.string;
        lyricPrinter.LoadData(filepath);
    }


    // Audio Player
    AudioPlayer player{};
    player.SetVolume(volume.value.number);

    {
        std::string filepath = std::string(RUNTIME_DIR) + song_file.value.string;
        player.LoadSoundFromFile(filepath);
    }

    // Run
    player.StartSound(start_delay.value.number);
    cursorFlash.Start();
    lyricPrinter.Start();

    do {
        for(auto& mutex : windowMutexes) { mutex.lock(); }
        Window::Update();
        for(auto& mutex : windowMutexes) { mutex.unlock(); }
    } while (getch() != 'q');

    // Stop
    cursorFlash.Stop();
    lyricPrinter.Stop();
}

//  ___ ___  ___
// | __/ _ \| __|
// | _| (_) | _|
// |___\___/|_|
//