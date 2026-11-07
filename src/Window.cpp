//  Copyright (C) 2026  Miradge/Sweat3rs
#include "Window.h"

#include <algorithm>

using namespace term;

Vec2& Vec2::operator+=(const Vec2& other)
{
    x += other.x;
    y += other.y;
    return *(this);
}

Vec2& Vec2::operator-=(const Vec2& other)
{
    x -= other.x;
    y -= other.y;
    return *(this);
}

Vec2& Vec2::operator*=(const Vec2& other)
{
    x *= other.x;
    y *= other.y;
    return *(this);
}

Vec2 Vec2::operator+(const Vec2& other)
{
    return Vec2{x + other.x, y + other.y};
}

Vec2 Vec2::operator-(const Vec2& other)
{
    return Vec2{x - other.x, y - other.y};
}

Vec2 Vec2::operator*(const Vec2& other)
{
    return Vec2{x * other.x, y * other.y};
}

uint32_t Window::s_numWindows{0};
uint32_t Window::s_counter{1};

bool Window::s_canChangeColors{false};
bool Window::s_hasColors{false};
bool Window::s_ignoreWarnings{false};

Error Window::s_globalErrorStatus{NONE};

Window::Window()
{
	if (s_globalErrorStatus != NONE) return;
    if (s_numWindows == 0) {
        Error error = initNCurses();
        if (error != NONE) {
            s_globalErrorStatus = error;
            return;
        }
    }
    s_numWindows++;
    m_colorIndex = s_counter++;

    m_position = {0,0};
    m_scale = {(uint16_t)COLS, (uint16_t)LINES};

    m_window = newwin(m_scale.y, m_scale.x, m_position.y, m_position.x);
    if (!m_window) {
		s_globalErrorStatus = TERMINAL_NOT_SUPPORTED;
		return;
	}
    Refresh();

    m_panel = new_panel(m_window);
	SetTop();

    UpdateScr();
}

Window::Window(uint32_t width, uint32_t height, uint32_t startx, uint32_t starty)
{
    if (s_globalErrorStatus != NONE) return;
    if (s_numWindows == 0) {
        Error error = initNCurses();
        if (error != NONE) {
            s_globalErrorStatus = error;
            return;
        }
    }
    s_numWindows++;
    m_colorIndex = s_counter++;

    m_position = {(int32_t)startx, (int32_t)starty};
    m_scale = {(int32_t)width, (int32_t)height};

    m_window = newwin(m_scale.y, m_scale.x, m_position.y, m_position.x);
    if (!m_window) {
		s_globalErrorStatus = TERMINAL_NOT_SUPPORTED;
		return;
	}
    Refresh();

    m_panel = new_panel(m_window);
	SetTop();

    UpdateScr();
}

Window::Window(Vec2 size, Vec2 position)
    : Window(size.x, size.y, position.x, position.y)
{

}

Window::~Window()
{
    s_numWindows--;

    if (s_numWindows == 0) {
        destroyNCurses();
    }
}

void Window::storeCursorPosition()
{
    m_cursorPosition = GetCursorPosition();
}

Error Window::Print(const std::string& string)
{
    wattron(m_window, COLOR_PAIR(m_colorIndex));
    wprintw(m_window, "%s", string.c_str());
    wattroff(m_window, COLOR_PAIR(m_colorIndex));

    storeCursorPosition();
    return NONE;
}

Error Window::AddCharacter(const char& ch)
{
    wattron(m_window, COLOR_PAIR(m_colorIndex));
    waddch(m_window, ch);
    wattroff(m_window, COLOR_PAIR(m_colorIndex));

    storeCursorPosition();
    return NONE;
}

Error Window::DrawBorder(chtype left, chtype right, chtype top, chtype bottom,
        chtype tlc, chtype trc, chtype blc, chtype brc)
{
    wattron(m_window, COLOR_PAIR(m_colorIndex));
    wborder(m_window, left, right, top, bottom, tlc, trc, blc, brc);
    wattroff(m_window, COLOR_PAIR(m_colorIndex));

    touchwin(m_window);
    return NONE;
}

Error Window::UpdateScr()
{
    update_panels();
    doupdate();
    return NONE;
}

void Window::SetTop()
{
    top_panel(m_panel);
}

Vec2 Window::GetCursorPosition()
{
    uint32_t x, y;
    getyx(m_window, y, x);
    return {(int32_t)x, (int32_t)y};
}

void Window::SetCursorPosition(Vec2 position) 
{
    wmove(m_window, position.y, position.x);
    m_cursorPosition = position;
}

Vec2 Window::GetWindowPosition()
{
    return m_position;
}

Vec2 Window::GetWindowSize()
{
    return m_scale;
}
void Window::MoveWindow(Vec2 delta)
{
    m_position += delta;
    move_panel(m_panel, m_position.y, m_position.x);
    update_panels();
}

void Window::SetWindowPosition(Vec2 position)
{
    m_position = position;
    move_panel(m_panel, m_position.y, m_position.x);
    update_panels();
}

void Window::Show()
{
    show_panel(m_panel);
    m_hidden = false;
}
void Window::Hide()
{
    hide_panel(m_panel);
    m_hidden = true;
}

void Window::FillBackgroundColor(uint16_t colorIndex)
{
    if (!s_hasColors) return;

    init_pair(m_colorIndex, m_currentColorPair[0], colorIndex);
    wbkgd(m_window, COLOR_PAIR(m_colorIndex));

    werase(m_window);
    touchwin(m_window);
}

void Window::SetColor(int16_t foreground, int16_t background)
{
    if (!s_hasColors) return; 

    m_currentColorPair[0] = (foreground == NO_CHANGE ? m_currentColorPair[0] : foreground);
    m_currentColorPair[1] = (background == NO_CHANGE ? m_currentColorPair[1] : background);
    init_pair(m_colorIndex, m_currentColorPair[0], m_currentColorPair[1]);
}

void Window::ChangeColorValue(uint16_t colorIndex, Color color)
{
    if (!s_hasColors || !s_canChangeColors) return;

    init_color(colorIndex, std::clamp((int)color.r, 0, 1000), std::clamp((int)color.g, 0, 1000), std::clamp((int)color.b, 0, 1000));
}

void Window::Clear()
{
    wclear(m_window);
}

void Window::SetScrollOk(bool value)
{
    scrollok(m_window, value);
}

void Window::Refresh() 
{
    wrefresh(m_window);
}

Vec2 Window::GetGlobalCursorPosition()
{
    if (s_numWindows == 0) {
        return Vec2{0,0};
    }

    uint32_t x, y;
    getmaxyx(stdscr, y, x);
    return {(int32_t)x, (int32_t)y};
}

Vec2 Window::GetTerminalSize()
{
    if (s_numWindows == 0) {
        return Vec2{0,0};
    }

    uint32_t x, y;
    getmaxyx(stdscr, y, x);
    return {(int32_t)x, (int32_t)y};
}

void Window::refreshSTDSCR()
{
    refresh();
}

Error Window::initNCurses() 
{
    initscr();
	if (stdscr == nullptr) {
		std::cerr << "Failed to initialize ncurses" << std::endl;
		return TERMINAL_NOT_SUPPORTED;
	}

    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    s_hasColors = has_colors();
    if (!s_hasColors && !s_ignoreWarnings) {
        destroyNCurses();
        std::cerr << "Your terminal doesn't support colors.\n" << std::endl;
        std::cerr << "To ignore this, start the program with the --ignore-warnings flag." << std::endl;
        return TERMINAL_NOT_SUPPORTED;
    }

    s_canChangeColors = can_change_color();
    if (!s_canChangeColors && !s_ignoreWarnings) {
        destroyNCurses();
        std::cerr << "Your terminal doesn't support custom colors. Colors might be inaccurate.\n";
        std::cerr << "To ignore this, start the program with the --ignore-warnings flag." << std::endl;
        return TERMINAL_NOT_SUPPORTED;
    }

    if (s_hasColors) start_color();
    return NONE;
}

Error Window::destroyNCurses()
{
    endwin();
    return NONE;
}