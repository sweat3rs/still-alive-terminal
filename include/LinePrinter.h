#pragma once

#include "Window.h"

#include <queue>
#include <chrono>

#include <thread>
#include <atomic>
#include <mutex>

namespace term 
{
    class CursorFlash {
        public:
            CursorFlash(Window& win, std::mutex& mutex);
            CursorFlash(Window& win, std::mutex& mutex, char flashCharacter, std::chrono::milliseconds halfPeriod);

            void setFlashCharacter(char flashCharacter);
            void setHalfPeriod(std::chrono::milliseconds halfPeriod);

            bool IsRunning() { return m_isRunning.load(); }

            void Start();
            void Stop();

        private:
            void worker();
            std::thread m_workerThread{};

            void displayCharacter();

            Window& m_window;
            std::mutex& m_windowMutex;

            char m_flashCharacter;
            std::chrono::milliseconds m_halfPeriod;

            bool m_isShowing = false;

            std::atomic<bool> m_isRunning = false;
            std::atomic<bool> m_SIGSTOP = false;

            static const std::chrono::milliseconds sc_timeoutQuanta;
    };

    struct LinePrintOut 
    {
        enum Modifier {
            NONE,
            NEWLINE,
            CLEAR
        };

        std::string line;
        Modifier lineModifier{NONE};

        std::chrono::milliseconds printTimePerCharacterMilliseconds{0};
        std::chrono::milliseconds startTimeMilliseconds{0};

        std::queue<char> charOutQueue{};

        // add timer
    };

    // Make sure the window + mutex live longer than the line printer
    class LinePrinter 
    {
        public:
            LinePrinter(Window& window, std::mutex& windowMutex, uint32_t millisecondsPerCharacter = 80);
            ~LinePrinter();

            void LoadData(const std::string& file);
            void PushLine(const LinePrintOut& line);

            bool IsRunning() { return m_isRunning.load(); }

            void Start();
            void Stop();

        private:
            void worker();
            std::thread m_workerThread{};

            Window& m_window;
            std::mutex& m_windowMutex;

            uint32_t m_millisecondsPerCharacterBase = 0;
            std::queue<LinePrintOut> m_linesOutQueue{};

            std::chrono::time_point<std::chrono::system_clock> m_startTime;

            std::atomic<bool> m_isRunning = false;
            std::atomic<bool> m_SIGSTOP = false;

            std::atomic<bool> m_errorState = false;

            static const std::chrono::milliseconds sc_timeoutQuanta;
    };
}