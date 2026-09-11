//  Copyright (C) 2026  Miradge/Sweat3rs
#pragma once

#ifdef __MINGW32__
    #include <ncurses/ncurses.h>
    #include <ncurses/panel.h>
#else
    #include <ncurses.h>
    #include <panel.h>
#endif

#include <iostream>

namespace term
{
    enum Error {
        NONE = 0,
        TERMINAL_NOT_SUPPORTED = 1
    };

    struct Color {
        uint16_t r;
        uint16_t g;
        uint16_t b;
    };

    struct Vec2 {
        uint32_t x;
        uint32_t y;
    };

    class Window {
        public:
            Window(); // defaults to fullscreen
            Window(uint32_t height, uint32_t width, uint32_t starty, uint32_t startx);
            ~Window();

            Error Print(const std::string& string);
            Error Update();

        private:
            void refreshWindow();
            Vec2 getTerminalSize();

            static Error initNCurses();
            static Error destroyNCurses();

            static uint32_t s_numWindows;
            static Error s_globalErrorStatus;

            PANEL* m_panel{nullptr};
            WINDOW* m_window{nullptr};
            Error m_windowErrorStatus{NONE};
    };
}