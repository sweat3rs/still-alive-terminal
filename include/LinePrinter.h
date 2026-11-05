#pragma once

#include "Window.h"

#include <queue>
#include <chrono>

#include <thread>
#include <atomic>
#include <mutex>

namespace term 
{
    struct LinePrintOut 
    {
        enum Modifier {
            NONE,
            NEWLINE,
            CLEAR
        };

        std::string line;
        std::queue<char> charOutQueue{};

        std::chrono::milliseconds printTimePerCharacterMilliseconds{0};
        std::chrono::milliseconds startTimeMilliseconds{0};

        Modifier lineModifier{NONE};

        // add timer
    };

    // Make sure the window + mutex live longer than the line printer
    class LinePrinter 
    {
        public:
            LinePrinter(Window& window, std::mutex& windowMutex, uint32_t millisecondsPerCharacter = 80);
            ~LinePrinter();

            void LoadData(const std::string& file);

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
            std::atomic<bool> m_errorState = false;

    };
}