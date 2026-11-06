#include "ImagePrinter.h"

#include "csv.h"
#include <cstdlib>

#define MILLISECONDS_SINCE_POINT(x) (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch() - (x).time_since_epoch()))

using namespace term;
using namespace std::chrono_literals;

const std::chrono::milliseconds ImagePrinter::sc_timeoutQuanta = 2ms;

ImagePrinter::ImagePrinter(Window& win, std::mutex& mutex)
    : m_window(win), m_windowMutex(mutex) {

}

void ImagePrinter::addImage(const std::string& name, const std::string& image)
{
    if (name.empty() || m_isRunning) return;
    
    m_images.emplace(name, image);
}

void ImagePrinter::loadTimingData(const std::string& file)
{
    if (m_isRunning) return;
    io::CSVReader<2, io::trim_chars<' ', '\t'>, io::double_quote_escape<',', '\"'>, io::throw_on_overflow, io::empty_line_comment> in(file);
    in.read_header(io::ignore_extra_column, "Print-Time", "Image-Name");
    std::string print_time, image_name;

    try {
        while(in.read_row(print_time, image_name)) {
            // process line
            ImagePrintData imagePrintData{image_name, 0ms};

            if (!print_time.empty() && print_time != "-") { imagePrintData.printTimeMilliseconds = std::chrono::milliseconds{(uint32_t)(atof(print_time.c_str())*1000.0)}; }

            m_imageQueue.emplace(std::move(imagePrintData));
        }
    } catch (std::exception& e) {
        m_errorState = true;
    }
}

void ImagePrinter::Start()
{
    if (m_errorState || m_isRunning) return;

    m_startTime = std::chrono::system_clock::now();
    m_workerThread = std::thread{&ImagePrinter::worker, this};
}
void ImagePrinter::Stop()
{
    if (m_errorState) return;

    m_SIGSTOP = true;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void ImagePrinter::worker() {
    m_isRunning = true;
    while(!m_imageQueue.empty() && !m_errorState && !m_SIGSTOP) {
        while(m_imageQueue.front().printTimeMilliseconds > MILLISECONDS_SINCE_POINT(m_startTime) && !m_SIGSTOP) {
            std::this_thread::sleep_for(sc_timeoutQuanta);
        }
        if (m_images.contains(m_imageQueue.front().imageName)) {
            std::scoped_lock<std::mutex> lock(m_windowMutex);
            m_window.Clear();
            m_window.SetCursorHome();
            m_window.Print(m_images.at(m_imageQueue.front().imageName));
        }
        m_imageQueue.pop();
    }
    m_SIGSTOP = false;
    m_isRunning = false;
}