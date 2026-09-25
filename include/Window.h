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

#define NO_CHANGE -1

namespace term
{
    enum Error {
        NONE = 0,
        TERMINAL_NOT_SUPPORTED = 1,
        USER_QUIT = 2
    };

    struct Color {
        uint16_t r;
        uint16_t g;
        uint16_t b;
    };

    struct Vec2 {
        uint32_t x;
        uint32_t y;

        Vec2 operator+(const Vec2& other);
        Vec2 operator-(const Vec2& other);
        Vec2 operator*(const Vec2& other);

        Vec2& operator+=(const Vec2& other);
        Vec2& operator-=(const Vec2& other);
        Vec2& operator*=(const Vec2& other);
    };

    class Window {
        public:
            Window(); // defaults to fullscreen
            Window(uint32_t height, uint32_t width, uint32_t starty, uint32_t startx);
            Window(Vec2 size, Vec2 position);
            ~Window();

            Error Print(const std::string& string);
            Error DrawBorder(chtype left, chtype right, chtype top, chtype bottom,
                chtype tlc = 0, chtype trc = 0, chtype blc = 0, chtype brc = 0);

            void SetTop();

            static Error GetGlobalErrorState() { return s_globalErrorStatus; }

            Vec2 GetCursorPosition();
            //void MoveCursor(Vec2 delta);
            void SetCursorPosition(Vec2 position);

            Vec2 GetWindowPosition();
            Vec2 GetWindowSize();
            void MoveWindow(Vec2 delta);
            void SetWindowPosition(Vec2 position);

            void FillBackgroundColor(uint16_t colorIndex);
            void SetColor(int16_t foreground, int16_t background);
            //static void ResetColors();

            static void FillBaseScreenColor(uint16_t colorIndex);

            void Clear();

            static void ChangeColorValue(uint16_t colorIndex, Color color);

            void Show();
            void Hide();

            static Error Update(); // Panels refresh
            static Vec2 GetGlobalCursorPosition();
            static Vec2 GetTerminalSize();

            static bool s_ignoreWarnings;

        private:
            void refreshWindow();

            static void globalRefresh(); // refresh()
            static Error initNCurses();
            static Error destroyNCurses();

            static uint32_t s_counter;
            static uint32_t s_numWindows;
            static Error s_globalErrorStatus;

            static bool s_hasColors;
            static bool s_canChangeColors;

            std::array<uint16_t, 2> m_currentColorPair{COLOR_WHITE, COLOR_BLACK};
            uint32_t m_colorIndex{0};

            Vec2 m_position{};
            Vec2 m_scale{};
            bool m_hidden{false};

            PANEL* m_panel{nullptr};
            WINDOW* m_window{nullptr};
            Error m_windowErrorStatus{NONE};
    };
}