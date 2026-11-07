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
#include "ImagePrinter.h"

#include <array>
#include <algorithm>

#include <filesystem>
#include <cstring>

#include <string>
#include <chrono>
#include <mutex>

#define LYRIC_WINDOW_SIZE 0.95
#define CREDITS_WINDOW_SIZE 0.6
#define BUFFER_SIZE 1

#define INFO_MESSAGE "Starting... Press \'q\' to quit."
#define LUA_CONFIG_TABLE "config"
#define LUA_IMAGE_TABLE "ascii_art"

std::string CONFIG_FILE{"config.lua"};

#ifdef SOURCE_RUNTIME_DIRECTORY
std::string RUNTIME_DIR{SOURCE_RUNTIME_DIRECTORY};
#else
std::string RUNTIME_DIR{};
#endif

// TODO: Add SIGWINCH handler, later

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
    
	auto print_speed = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "lyric_print_speed");
    auto credits_print_speed = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "credits_print_speed");
    
	auto icons_file = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "icons_file");
    
	auto icons_timing = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "icons_timing");
    auto lyrics_file = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "lyrics_file");
    auto credits_file = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "credits_file");

    if (volume.type != Lua::NUMBER || song_file.type != Lua::STRING || start_delay.type != Lua::NUMBER
        || print_speed.type != Lua::NUMBER || credits_print_speed.type != Lua::NUMBER || icons_file.type != Lua::STRING 
        || icons_timing.type != Lua::STRING || lyrics_file.type != Lua::STRING || credits_file.type != Lua::STRING) {
        std::cerr << "Lua config file parsing failed!" << std::endl;
        return 1;
    }

	auto ignore_warnings = lua.GetFieldFromTable(LUA_CONFIG_TABLE, "ignore_warnings");
	if (ignore_warnings.type == Lua::BOOL) {
		Window::s_ignoreWarnings = Window::s_ignoreWarnings || ignore_warnings.value.boolean;
	}

    // Images
    std::array<std::string, 11> imageNames = {
        "empty",
        "aperture",
        "radiation",
        "atom",
        "broken_heart",
        "fire",
        "checkmark",
        "explosion",
        "black_mesa",
        "cake",
        "glados"
    };

    {
        std::string filepath = std::string{RUNTIME_DIR} + icons_file.value.string;
        lua.AccessTableFromFile(filepath, LUA_IMAGE_TABLE);
    }

    auto image_width = lua.GetFieldFromTable(LUA_IMAGE_TABLE, "width");
    auto image_height = lua.GetFieldFromTable(LUA_IMAGE_TABLE, "height");

    if (image_width.type != Lua::NUMBER || image_height.type != Lua::NUMBER) {
        std::cerr << "Lua config file parsing failed!" << std::endl;
        return 1;
    }
    Vec2 imageSize{(int32_t)image_width.value.number, (int32_t)image_height.value.number};

    // Windows
    std::array<Window, 6> windows;
    // 0 - main
    // 1, 2 - borders
    // 3, 4, 5 - 3 windows

    windows[0] = Window{};
    if (Window::GetGlobalErrorState() != NONE) return 1;

    Vec2 termSize = Window::GetTerminalSize();
    // Term min size is <96, 27> 
    if ((termSize.x < 96 || termSize.y < 27) && !Window::s_ignoreWarnings) {
        std::cerr << "Terminal window is too small, min size is <96, 27>" << std::endl;
        return 1;
    }

    uint32_t windowWidths = (termSize.x/2) - 1 - BUFFER_SIZE;
    uint32_t lyricWindowHeight = termSize.y*LYRIC_WINDOW_SIZE - BUFFER_SIZE;
    uint32_t creditWindowHeight = std::min(uint32_t(termSize.y*LYRIC_WINDOW_SIZE*CREDITS_WINDOW_SIZE - BUFFER_SIZE), uint32_t(termSize.y - imageSize.y - BUFFER_SIZE*2));
    
    windows[1] = Window{windowWidths, lyricWindowHeight, BUFFER_SIZE, BUFFER_SIZE};
    windows[2] = Window{windowWidths, creditWindowHeight, (windowWidths + 2) + BUFFER_SIZE, BUFFER_SIZE};
    windows[3] = Window{(uint32_t)imageSize.x, (uint32_t)imageSize.y + BUFFER_SIZE*2, ((windowWidths + 2) + BUFFER_SIZE) + (windowWidths/2 - imageSize.x/2), (creditWindowHeight + BUFFER_SIZE) + std::max(((termSize.y - creditWindowHeight + BUFFER_SIZE)/2 - imageSize.y/2), (uint32_t)0)};

    windows[4] = Window{windowWidths - 2, lyricWindowHeight - 2, BUFFER_SIZE + 1, BUFFER_SIZE + 1};
    windows[5] = Window{windowWidths - 2, creditWindowHeight - 2, (windowWidths + 2) + BUFFER_SIZE + 1, BUFFER_SIZE + 1};

    Window::ChangeColorValue(COLOR_BLACK, Color{30,30,30});
    Window::ChangeColorValue(COLOR_YELLOW, Color{1000, 700, 0});

    for (auto& win : windows) {
        win.FillBackgroundColor(COLOR_BLACK);
        win.SetColor(COLOR_YELLOW, COLOR_BLACK);
		win.Refresh();
    }

    Window::UpdateScr();

    windows[1].DrawBorder('|', '|', '-', '-', ' ', ' ', ' ', ' ');
    windows[2].DrawBorder('|', '|', '-', '-', ' ', ' ', ' ', ' ');
    //windows[3].DrawBorder('|', '|', '-', '-', ' ', ' ', ' ', ' ');

    std::mutex windowMutex{};

    // Lyric animations
    windows[4].SetCursorHomePos({1, 0});

    CursorFlash cursorFlashLyric{windows[4], windowMutex, '_', 600ms};
    LinePrinter lyricPrinter(windows[4], windowMutex, print_speed.value.number);
    {
        std::string filepath = std::string(RUNTIME_DIR) + lyrics_file.value.string;
        lyricPrinter.LoadData(filepath);
    }

    // Credit names
    windows[5].SetCursorHomePos({1, windows[5].GetWindowSize().y - 1});
    windows[5].SetCursorHome();
    windows[5].SetScrollOk(true);

    CursorFlash cursorFlashCredit{windows[5], windowMutex, '_', 300ms};
    LinePrinter creditsPrinter(windows[5], windowMutex, credits_print_speed.value.number);
    {
        std::string filepath = std::string(RUNTIME_DIR) + credits_file.value.string;
        creditsPrinter.LoadData(filepath);
        
        uint32_t numExtraLines = (uint32_t)(windows[5].GetWindowSize().y/2) - 1;
        LinePrintOut line{};
        line.line = "  ";
        line.lineModifier = LinePrintOut::NEWLINE;
        line.printTimePerCharacterMilliseconds = std::chrono::milliseconds{(uint32_t)(credits_print_speed.value.number*2.0)};
        for (uint32_t i = 0; i < numExtraLines; i++) { creditsPrinter.PushLine(line); }
    }

    // Ascii Art
    ImagePrinter imagePrinter{windows[3], windowMutex};
    for (auto& name : imageNames) {
        auto image = lua.GetFieldFromTable(LUA_IMAGE_TABLE, name);
        if (image.type == Lua::STRING) {
            imagePrinter.addImage(name, image.value.string);
        }
    }
    {
        std::string filepath = std::string(RUNTIME_DIR) + icons_timing.value.string;
        imagePrinter.loadTimingData(filepath);
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
    cursorFlashLyric.Start();
    cursorFlashCredit.Start();

    lyricPrinter.Start();
    creditsPrinter.Start();
    imagePrinter.Start();

    auto c = getch();
    do {
        std::scoped_lock<std::mutex> lock(windowMutex);
		for (auto& win : windows) win.Refresh();
        Window::UpdateScr();
    } while (c = getch(), c != 'q' && c != KEY_RESIZE && lyricPrinter.IsRunning());

    // Stop
    lyricPrinter.Stop();
    creditsPrinter.Stop();
    imagePrinter.Stop();
    cursorFlashLyric.Stop();
    cursorFlashCredit.Stop();
}

//  ___ ___  ___
// | __/ _ \| __|
// | _| (_) | _|
// |___\___/|_|
//