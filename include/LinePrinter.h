#pragma once

#include "Window.h"
#include <queue>

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

        uint32_t printTimePerCharacterMilliseconds{0};
        Modifier lineModifier{NONE};

        // add timer
    };

    class LinePrinter 
    {
        public:
            LinePrinter(Window& window);
            ~LinePrinter();

            void LoadData(const std::string& file);

            void Start();
            void Stop();

        private:
            Window& m_window;
            std::queue<LinePrintOut> m_linesOutQueue{};

    };
}