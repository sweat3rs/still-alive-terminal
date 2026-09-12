//  Copyright (C) 2026  Miradge/Sweat3rs
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

            void SetVolume(double volume, bool isNorm = false); // set isNorm to true for 0-1, else it will be 0-100

            ma_result GetErrorState() { return m_errorState; }

        private:
            ma_engine m_audioEngine{};
            ma_engine_config m_config{};

            ma_sound m_sound{};

            ma_result m_errorState;
    };
}