//  Copyright (C) 2026  Miradge/Sweat3rs
#include "Window.h"

namespace term
{
    uint32_t Window::s_numWindows{0};
    Error Window::s_globalErrorStatus{NONE};

    Window::Window()
    {
        if (s_numWindows == 0) {
            Error error = initNCurses();
            if (error != NONE) {
                s_globalErrorStatus = error;
                return;
            }
        }

        m_window = newwin(LINES, COLS, 0, 0);
        refresh();
        
        refreshWindow();

        s_numWindows++;
    }

    Window::Window(uint32_t height, uint32_t width, uint32_t starty, uint32_t startx)
    {
        if (s_numWindows == 0) {
            Error error = initNCurses();
            if (error != NONE) {
                s_globalErrorStatus = error;
                return;
            }
        }

        m_window = newwin(height, width, starty, startx);
        refresh();
        
        refreshWindow();

        s_numWindows++; 
    }

    Window::~Window()
    {
        s_numWindows--;

        if (s_numWindows == 0) {
            destroyNCurses();
        }
    }

        Error Window::Print(const std::string& string)
    {
        wprintw(m_window, string.c_str());
        refreshWindow();
        return NONE;
    }

    Error Window::Update()
    {
        doupdate();
    }

    void Window::refreshWindow() {
        wrefresh(m_window);
    }

    Vec2 Window::getTerminalSize() {
        uint32_t x, y;
        getmaxyx(m_window, y, x);
        return {x, y};
    }

    Error Window::initNCurses() 
    {
        initscr();
        noecho();
        cbreak();
        keypad(stdscr, TRUE);
        curs_set(0);

        if (has_colors() == false || can_change_color() == false) {
            endwin();
            std::cerr << "Your terminal doesn't support colors." << std::endl;
            return TERMINAL_NOT_SUPPORTED;
        }

        start_color();
        return NONE;
    }

    Error Window::destroyNCurses()
    {
        endwin();
        return NONE;
    }
}