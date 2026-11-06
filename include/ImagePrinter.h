#pragma once

#include "Window.h"

#include <thread>
#include <mutex>
#include <chrono>
#include <queue>
#include <unordered_map>

namespace term 
{
    struct ImagePrintData
    {
        std::string imageName;
        std::chrono::milliseconds printTimeMilliseconds = std::chrono::milliseconds{0};
    };

    class ImagePrinter 
    {
        public:
            ImagePrinter(Window& win, std::mutex& mutex);

            bool IsRunning() { return m_isRunning.load(); }

            void addImage(const std::string& name, const std::string& image);
            void loadTimingData(const std::string& file);

            void Start();
            void Stop();

        private:
            std::thread m_workerThread;
            void worker();

            Window& m_window;
            std::mutex& m_windowMutex;

            std::atomic<bool> m_isRunning = false;
            std::atomic<bool> m_SIGSTOP = false;

            std::atomic<bool> m_errorState = false;

            std::chrono::time_point<std::chrono::system_clock> m_startTime;

            std::unordered_map<std::string, std::string> m_images;
            std::queue<ImagePrintData> m_imageQueue;

            static const std::chrono::milliseconds sc_timeoutQuanta;
    };

}