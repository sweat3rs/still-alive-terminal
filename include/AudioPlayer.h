#pragma once

#include "miniaudio.h"

#include <string>

namespace term {
    class AudioPlayer 
    {
        public:
            AudioPlayer();
            ~AudioPlayer();

            void LoadSoundFromFile(const std::string& file);

            void StartSound(double delaySeconds = 0.0);
            void StopSound(double delaySeconds = 0.0);

            ma_result GetErrorState() { return m_errorState; }

        private:
            ma_engine m_audioEngine{};
            ma_engine_config m_config{};

            ma_sound m_sound{};

            ma_result m_errorState;
    };
}