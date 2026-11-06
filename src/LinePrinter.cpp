#include "LinePrinter.h"

#include "csv.h"
#include <cstdlib>

using namespace std::literals::chrono_literals;

#define MILLISECONDS_SINCE_POINT(x) std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch() - (x).time_since_epoch())

using namespace term;



const std::chrono::milliseconds LinePrinter::sc_timeoutQuanta = 1ms;
const std::chrono::milliseconds CursorFlash::sc_timeoutQuanta = 4ms;

CursorFlash::CursorFlash(Window& win, std::mutex& mutex)
    : m_window(win), m_windowMutex(mutex) {

}
CursorFlash::CursorFlash(Window& win, std::mutex& mutex, char flashCharacter, std::chrono::milliseconds halfPeriod)
    : m_window(win), m_windowMutex(mutex), m_flashCharacter(flashCharacter), m_halfPeriod(halfPeriod) {

}

void CursorFlash::setFlashCharacter(char flashCharacter)
{
    std::scoped_lock<std::mutex> lock(m_windowMutex);
    m_flashCharacter = flashCharacter;
}
void CursorFlash::setHalfPeriod(std::chrono::milliseconds halfPeriod)
{
    std::scoped_lock<std::mutex> lock(m_windowMutex);
    m_halfPeriod = halfPeriod;
}

void CursorFlash::Start()
{
    if (m_isRunning) return;
    m_workerThread = std::thread(&CursorFlash::worker, this);
}

void CursorFlash::Stop()
{
    if (!m_isRunning) return;

    m_SIGSTOP = true;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void CursorFlash::displayCharacter() 
{
    if (m_isShowing) {
        std::scoped_lock<std::mutex> lock(m_windowMutex);
            Window::Update();
            m_window.AddCharacter(m_flashCharacter);
            auto pos = m_window.GetCursorPosition();
            pos.x -= 1;
            m_window.SetCursorPosition(pos);
    }
    else {
        std::scoped_lock<std::mutex> lock(m_windowMutex);
        Window::Update();
        m_window.AddCharacter(' ');
        auto pos = m_window.GetCursorPosition();
        pos.x -= 1;
        m_window.SetCursorPosition(pos);
    }
}

void CursorFlash::worker()
{
    m_isRunning = true;

    auto start = std::chrono::system_clock::now();
    while (!m_SIGSTOP) {
        if (MILLISECONDS_SINCE_POINT(start) > m_halfPeriod) {
            m_isShowing = !m_isShowing;
            start = std::chrono::system_clock::now();
        }
        
        displayCharacter();
        std::this_thread::sleep_for(sc_timeoutQuanta);
    }
    m_SIGSTOP = false;
    m_isRunning = false;
}



LinePrinter::LinePrinter(Window& window, std::mutex& windowMutex, uint32_t millisecondsPerCharacter)
    : m_window(window), m_windowMutex(windowMutex), m_millisecondsPerCharacterBase(millisecondsPerCharacter)
{

}

LinePrinter::~LinePrinter()
{

}

void LinePrinter::LoadData(const std::string& file)
{
    if (m_isRunning) return;
    io::CSVReader<4, io::trim_chars<' ', '\t'>, io::double_quote_escape<',', '\"'>, io::throw_on_overflow, io::empty_line_comment> in(file);
    in.read_header(io::ignore_extra_column, "Start-Time", "Speed", "Text", "operator");
    std::string start_time, print_speed_mult, text, line_operator;

    try {
        while(in.read_row(start_time, print_speed_mult, text, line_operator)) {
            // process line
            LinePrintOut line;

            if (start_time.empty() || start_time == "_") { line.startTimeMilliseconds = 0ms; } 
            else { line.startTimeMilliseconds = std::chrono::milliseconds{(uint32_t)(atof(start_time.c_str())*1000.0)}; }

            if (print_speed_mult.empty() || atof(print_speed_mult.c_str()) <= 0.0) {
                line.printTimePerCharacterMilliseconds = std::chrono::milliseconds{m_millisecondsPerCharacterBase};
            } 
            else {
                line.printTimePerCharacterMilliseconds = std::chrono::milliseconds{(uint32_t)((double)m_millisecondsPerCharacterBase * atof(print_speed_mult.c_str()))};
            }

            line.line = std::move(text);

            if (line_operator == "newline") { line.lineModifier = LinePrintOut::Modifier::NEWLINE; }
            else if (line_operator == "clear") { line.lineModifier = LinePrintOut::Modifier::CLEAR; }
            else { line.lineModifier = LinePrintOut::Modifier::NONE; }

            for (char character : line.line) { line.charOutQueue.push(character); }

            m_linesOutQueue.emplace(std::move(line));
        }
    } catch (std::exception& e) {
        m_errorState = true;
    }
}

void LinePrinter::Start()
{
    if (m_errorState) return;
    m_window.SetCursorHome();

    m_startTime = std::chrono::system_clock::now();
    m_workerThread = std::thread(&LinePrinter::worker, this);
}

void LinePrinter::Stop()
{
    if (m_errorState || !m_isRunning) return;

    m_SIGSTOP = true;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    
}

//TODO: Add precise time checking and adjustment
void LinePrinter::worker()
{
    m_isRunning = true;

    while (!m_linesOutQueue.empty() && !m_errorState && !m_SIGSTOP) {
        if (m_linesOutQueue.front().startTimeMilliseconds != 0ms) {
            // wait
            while (m_linesOutQueue.front().startTimeMilliseconds > MILLISECONDS_SINCE_POINT(m_startTime) && !m_SIGSTOP) {
                std::this_thread::sleep_for(sc_timeoutQuanta);
            }
        }
        
        while (!m_linesOutQueue.front().charOutQueue.empty() && !m_SIGSTOP) {
            {
                std::scoped_lock<std::mutex> lock(m_windowMutex);
                m_window.AddCharacter(m_linesOutQueue.front().charOutQueue.front());
                m_linesOutQueue.front().charOutQueue.pop();
            }
            auto start = std::chrono::system_clock::now();
            if (m_linesOutQueue.front().charOutQueue.size() > 0) {
                while (m_linesOutQueue.front().printTimePerCharacterMilliseconds > MILLISECONDS_SINCE_POINT(start) && !m_SIGSTOP) {
                    std::this_thread::sleep_for(sc_timeoutQuanta);
                }
            }
        }

        switch (m_linesOutQueue.front().lineModifier) {
            case LinePrintOut::Modifier::NEWLINE:
                {
                    std::scoped_lock<std::mutex> lock(m_windowMutex);
                    m_window.AddCharacter(' ');
                    m_window.SetCursorPosition(Vec2{m_window.GetHomeCursorPos().x, (m_window.GetCursorPosition().y + 1)}); // newline
                }
                break;
            case LinePrintOut::Modifier::CLEAR:
                {
                    std::scoped_lock<std::mutex> lock(m_windowMutex);
                    m_window.Clear();
                    m_window.SetCursorHome();
                }
                break;
            default:
                break;
        }

        m_linesOutQueue.pop();
    }
    m_SIGSTOP = false;
    m_isRunning = false;
}