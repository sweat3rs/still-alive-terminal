#include "LinePrinter.h"

#include "csv.h"
#include <cstdlib>

using namespace std::literals::chrono_literals;

#define MILLISECONDS_SINCE_POINT(x) (std::chrono::duration_cast<std::chrono::milliseconds>((x).time_since_epoch()))

using namespace term;

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

            if (start_time.empty()) { line.startTimeMilliseconds = 0ms; } 
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
    m_workerThread.detach();
}

void LinePrinter::Stop()
{
    if (m_errorState || !m_isRunning) return;

    m_errorState = true;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    m_errorState = false;
}

//TODO: Add precise time checking and adjustment
void LinePrinter::worker()
{
    while (!m_linesOutQueue.empty() && !m_errorState) {
        
        while (!m_linesOutQueue.front().charOutQueue.empty()) {
            {
                std::scoped_lock<std::mutex> lock(m_windowMutex);
                m_window.AddCharacter(m_linesOutQueue.front().charOutQueue.front());
                m_linesOutQueue.front().charOutQueue.pop();
            }
            if (m_linesOutQueue.front().charOutQueue.size() > 0) std::this_thread::sleep_for(std::chrono::milliseconds(m_linesOutQueue.front().printTimePerCharacterMilliseconds));
        }

        switch (m_linesOutQueue.front().lineModifier) {
            case LinePrintOut::Modifier::NEWLINE:
                {
                    std::scoped_lock<std::mutex> lock(m_windowMutex);
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

        if (m_linesOutQueue.front().startTimeMilliseconds != 0ms) {
            // wait
            if(m_linesOutQueue.front().startTimeMilliseconds > MILLISECONDS_SINCE_POINT(m_startTime)) {
                std::this_thread::sleep_for(m_linesOutQueue.front().startTimeMilliseconds - MILLISECONDS_SINCE_POINT(m_startTime));
            }
        }

        m_linesOutQueue.pop();
    }
}